<?php

/**
 * @file
 * Administrative pages.
 */

/**
 * Form builder for API config.
 */
function apple_news_settings_api_form($form, &$form_state) {

  $form['apple_news_variable_api_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint'),
    '#default_value' => variable_get('apple_news_variable_api_endpoint', ''),
  );

  $form['apple_news_variable_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Key'),
    '#default_value' => variable_get('apple_news_variable_api_key', ''),
  );

  $form['apple_news_variable_api_secret'] = array(
    '#type' => 'password',
    '#title' => t('Secret'),
    '#default_value' => variable_get('apple_news_variable_api_secret', ''),
  );

  $form['apple_news_variable_api_channels'] = array(
    '#type' => 'textarea',
    '#title' => t('Channels'),
    '#description' => t("Separate channel id's with whitespace"),
    '#default_value' => implode("\n",
      variable_get('apple_news_variable_api_channels', array())),
  );

  $form = system_settings_form($form);
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete configuration')
  );
  $form['#validate'] = array('apple_news_settings_api_form_validate');
  $form['#submit'] = array('apple_news_settings_api_form_submit');
  return $form;
}

/**
 * Form validation handler.
 */
function apple_news_settings_api_form_validate($form, &$form_state) {
  switch ($form_state['triggering_element']['#parents'][0]) {

    case 'delete':
      break;

    default:
      foreach (array(
                 'apple_news_variable_api_endpoint',
                 'apple_news_variable_api_key',
                 'apple_news_variable_api_secret',
                 'apple_news_variable_api_channels',
               ) as $key
      ) {
        if (empty(trim($form_state['values'][$key]))) {
          form_set_error($key, t('Missing required value "@title".', array('@title' => $form[$key]['#title'])));
        }
      }
      break;

  }
}

/**
 * Form submit handler.
 */
function apple_news_settings_api_form_submit($form, &$form_state) {
  switch ($form_state['triggering_element']['#parents'][0]) {

    case 'delete':
      foreach (array(
                 'apple_news_variable_api_endpoint',
                 'apple_news_variable_api_key',
                 'apple_news_variable_api_secret',
               ) as $key
      ) {
        variable_set($key, '');
      }
      $key = 'apple_news_variable_api_channels';
      variable_set($key, array());
      drupal_set_message(t('The configuration options have been deleted.'));
      break;

    default:
      foreach (array(
                 'apple_news_variable_api_endpoint',
                 'apple_news_variable_api_key',
                 'apple_news_variable_api_secret',
               ) as $key
      ) {
        $value = $form_state['values'][$key];
        if (is_array($value) && isset($form_state['values']['array_filter'])) {
          $value = array_keys(array_filter($value));
        }
        variable_set($key, $value);
      }
      $key = 'apple_news_variable_api_channels';
      $value = array_filter(preg_split('/\s+/', $form_state['values'][$key]));
      variable_set($key, $value);
      drupal_set_message(t('The configuration options have been saved.'));
      foreach ($value as $channel) {
        if ($response = apple_news_get_channel($channel)) {
          $msg = t('Successfully connected to the channel "@channel".', array(
            '@channel' => $response->data->name,
          ));
          drupal_set_message($msg, 'status');
        }
        else {
          $msg = t('There was an error communicating with the server.');
          drupal_set_message($msg, 'error');
        }
      }
      break;

  }
}

/**
 * Form builder for node export config.
 */
function apple_news_settings_exports_form($form, &$form_state) {

  $exports = apple_news_exports();

  $header = array(
    'name' => array('data' => t('Name')),
    'description' => array('data' => t('Description')),
    'module' => array('data' => t('Module')),
    'enabled' => array('data' => t('Enabled')),
    'status' => array('data' => t('Status')),
    'lastrun' => array('data' => t('Last Run')),
  );

  $rows = array();
  foreach ($exports as $key => $info) {
    $query = db_select('apple_news_log', 'l')
      ->fields('l', array('starttime', 'endtime'))
      ->condition('l.module', $info['module'])
      ->condition('l.machine_name', $info['machine_name'])
      ->orderBy('l.starttime', 'DESC');
    $log = $query->range(0, 1)->execute()->fetchAssoc();

    $name = empty($info['name']) ? $info['machine_name'] : $info['name'];
    $module = system_get_info('module', $info['module'])['name'];
    $enabled = empty($info['#status']['enabled']) ? t('no') : t('yes');
    $status = empty($log['endtime']) ? (empty($log['starttime']) ? '' : t('incomplete')) : t('complete');
    $lastrun = empty($log['starttime']) ? t('never') : (int) $log['starttime'];
    $rows[$key] = array(
      'name' => $name,
      'description' => $info['description'],
      'module' => $module,
      'enabled' => $enabled,
      'status' => $status,
      'lastrun' => $lastrun,
    );
  }

  $form['exports'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#tree' => TRUE,
    '#empty' => t('No exports are defined.'),
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'action' => array(
      '#type' => 'select',
      '#title' => t('Action'),
      '#options' => array(
        'enable' => t('Enable'),
        'disable' => t('Disable'),
        'run' => t('Run'),
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    ),
  );

  $form['#submit'][] = 'apple_news_settings_exports_form_submit';
  $form['#theme'] = 'system_settings_form';

  return $form;
}

/**
 * Form submit validator.
 */
function apple_news_settings_exports_form_validate($form, &$form_state) {

  $selected = array_filter($form_state['values']['exports']);
  if (empty($selected)) {
    form_set_error('exports', 'Select at lease one export.');
  }

}

/**
 * Form submit handler.
 */
function apple_news_settings_exports_form_submit($form, &$form_state) {

  $exports = apple_news_exports();
  $selected = array_map(function($key) use(&$exports) {
    return $exports[$key];
  }, array_filter($form_state['values']['exports']));

  switch ($form_state['values']['action']) {

    case 'enable':
      foreach ($selected as $info) {
        apple_news_export_enable($info['module'], $info['machine_name'], TRUE);
      }
      break;

    case 'disable':
      foreach ($selected as $info) {
        apple_news_export_enable($info['module'], $info['machine_name'], FALSE);
      }
      break;

    case 'run':
      drupal_set_message('Run not yet implemented');
      break;

  }
}
