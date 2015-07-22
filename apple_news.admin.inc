<?php

/**
 * @file
 * Administrative pages.
 */

/**
 * Form builder for API config.
 */
function apple_news_settings_api_credentials_form($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  $form['credentials'] = array(
    '#type' => 'fieldset',
    '#title' => t('Apple News Credentials')
  );

  $form['credentials']['apple_news_variable_api_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('API Endpoint URL'),
    '#default_value' => variable_get('apple_news_variable_api_endpoint', ''),
    '#description' => t('Push API endpoint URL'),
  );

  $form['credentials']['apple_news_variable_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key ID'),
    '#default_value' => variable_get('apple_news_variable_api_key', ''),
    '#description' => t('Push API Key ID'),
  );

  $form['credentials']['apple_news_variable_api_secret'] = array(
    '#type' => 'password',
    '#title' => t('API Secret Key'),
    '#default_value' => variable_get('apple_news_variable_api_secret', ''),
    '#description' => t('Push API Secret Key'),
  );

  $form['save_config'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array('apple_news_settings_api_credentials_form_submit'),
    '#validate' => array('apple_news_settings_api_credentials_form_validate'),
  );

  $form['delete_config'] = array(
    '#markup' => l(t('Delete configuration'), 'admin/config/content/apple-news/settings/delete'),
  );

  return $form;
}

/**
 * Form validation handler.
 */
function apple_news_settings_api_credentials_form_validate($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  foreach (apple_news_settings_fields() as $key) {
    if (isset($form['credentials'][$key]['#title'])) {
      $value = trim($form_state['values'][$key]);
      if ($value === '') {
        form_set_error($key, t('Missing required value "@title".', array('@title' => $form['credentials'][$key]['#title'])));
      }
    }
  }
}

/**
 * Form submit handler.
 */
function apple_news_settings_api_credentials_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  foreach (apple_news_settings_fields() as $key) {
    if (isset($form['credentials'][$key]['#title'])) {
      $value = $form_state['values'][$key];
      if (is_array($value) && isset($form_state['values']['array_filter'])) {
        $value = array_keys(array_filter($value));
      }
      variable_set($key, $value);
    }
  }
  drupal_set_message(t('The configuration options have been saved. Please <a href="!link">add Feed ID</a> to verify connection and start posting content to Apple News.', array('!link' => url('admin/config/content/apple-news/settings/channels'))));
}

/**
 * Delete Apple News configuration w/ confirmation.
 */
function apple_news_settings_delete_configuration_form($form, &$form_state) {
  return confirm_form(
    $form,
    t("Do you really want to delete current Apple News configuration?"),
    'admin/config/content/apple-news/settings',
    t("You will no longer be able to publish content to Apple News. This action cannot be undone."),
    t("Yes, delete configuration")
  );
}

/**
 * Form submit handler.
 */
function apple_news_settings_delete_configuration_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  foreach (apple_news_settings_fields() as $key) {
    variable_set($key, '');
  }
  variable_set('apple_news_variable_api_channels', array());
  drupal_set_message(t('The configuration options have been deleted.'));
  drupal_goto('admin/config/content/apple-news/settings');
}

/**
 * Form builder for API config.
 */
function apple_news_settings_channels_form() {
  module_load_include('inc', 'apple_news');

  $channels = [];
  $api_channels = variable_get('apple_news_variable_api_channels', array());
  foreach ($api_channels as $channel_id => $info) {
    $channels[] = array(
      'name' => $info['name'],
      'id' => $channel_id,
      'operations' => l('Delete', 'admin/config/content/apple-news/settings/channels/' . $channel_id . '/delete')
    );
  }

  $form['channels'] = array(
    '#markup' => '<br/>' . theme('table',
      array(
        'header' => array(
          t('Feed Name'), // Channel Name
          t('Feed ID'), // Channel ID
          t('Operations'),
        ),
        'rows' => $channels,
        'empty' => t('Please add at least one feed in order to be able to publish content to Apple News.'),
      )
    )
  );

  $form['apple_news_channel_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Feed ID'), // Channel ID
    '#description' => t("Provide a Feed ID"),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add new feed'),
  );

  return $form;
}

/**
 * Form submit handler.
 */
function apple_news_settings_channels_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news', 'push_api');

  $channel_id = $form_state['values']['apple_news_channel_id'];
  $channels = variable_get('apple_news_variable_api_channels', array());

  if ($response = apple_news_get_channel($channel_id)) {
    // Save settings.
    if (!isset($response->errors)) {
      $new_channel = [
        $channel_id => (array) $response->data,
      ];
      variable_set('apple_news_variable_api_channels', array_merge($channels, $new_channel));
      $msg = t('Successfully connected to the feed "@channel".', array(
        '@channel' => $response->data->name,
      ));
      drupal_set_message($msg, 'status');
    }
    else {
      $msg = t('There was an error communicating with the server. Please make sure you have correct Feed ID and Apple News credentials.');
      drupal_set_message($msg, 'error');
    }
  }
  else {
    $msg = t('There was an error communicating with the server.');
    drupal_set_message($msg, 'error');
  }

  drupal_set_message(t('The configuration options have been saved.'));

}

/**
 * Delete channel ID with confirmation.
 */
function apple_news_settings_delete_channel_form($form, &$form_state, $channel_id) {
  $channels = variable_get('apple_news_variable_api_channels', array());
  $form['channel_id'] = array(
    '#type' => 'hidden',
    '#value' => $channel_id,
  );
  return confirm_form(
    $form,
    t("Do you really want to delete this feed?"),
    'admin/config/content/apple-news/settings/channels',
    t("You will no long be able to publish content to <em>@name</em> feed. This action cannot be undone.",
      array('@name' => $channels[$channel_id]['name'])
    ),
    t("Yes, delete")
  );
}

/**
 * Form submit handler.
 */
function apple_news_settings_delete_channel_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  $channels = variable_get('apple_news_variable_api_channels', array());
  $channel_id = $form_state['values']['channel_id'];
  if (!isset($channels[$channel_id])) {
    drupal_set_message(t('You are trying to remove non-existing feed.'), 'warning');
  }
  else {
    drupal_set_message(t('<em>@name</em> feed successfully removed.', array('@name' => $channels[$channel_id]['name'])));
    unset($channels[$channel_id]);
    // Remove channel ID from everywhere.
    apple_news_cleanup($channel_id);
    variable_set('apple_news_variable_api_channels', $channels);
    drupal_goto('admin/config/content/apple-news/settings/channels');
  }

}

/**
 * Form builder for node export config.
 */
function apple_news_settings_exports_form($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  $header = array(
    'name' => array('data' => t('Name')),
    'description' => array('data' => t('Description')),
    'module' => array('data' => t('Module')),
    'enabled' => array('data' => t('Enabled')),
    'status' => array('data' => t('Status')),
    'lastexport' => array('data' => t('Last Export')),
    'edit' => array('data' => ''),
  );

  $rows = array();
  foreach (apple_news_exports() as $key => $info) {
    $query = db_select('apple_news_log', 'l')
      ->fields('l', array('starttime', 'endtime'))
      ->condition('l.module', $info['module'])
      ->condition('l.machine_name', $info['machine_name'])
      ->orderBy('l.starttime', 'DESC');
    $log = $query->range(0, 1)->execute()->fetchAssoc();

    $name = empty($info['name']) ? $info['machine_name'] : $info['name'];
    $module = system_get_info('module', $info['module']);
    $module = $module['name'];
    $enabled = empty($info['#status']['enabled']) ? t('no') : t('yes');
    $status = empty($log['endtime']) ? (empty($log['starttime']) ? '' : t('incomplete')) : t('complete');
    $lastexport = empty($log['starttime']) ? t('never') : format_date($log['starttime'], 'short');
    $edit = l(t('edit'),
      'admin/config/content/apple-news/bulk-export/' . $key . '/edit',
      array(
        'query' => array(
          'destination' => 'admin/config/content/apple-news/bulk-export',
        ),
      )
    );
    $rows[$key] = array(
      'name' => $name,
      'description' => $info['description'],
      'module' => $module,
      'enabled' => $enabled,
      'status' => $status,
      'lastexport' => $lastexport,
      'edit' => $edit,
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
        'export' => t('Export'),
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
 * Form validatation handler.
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

  module_load_include('inc', 'apple_news', 'push_api');
  module_load_include('inc', 'apple_news');

  $exports = apple_news_exports();
  $selected = array_map(function($key) use(&$exports) {
      return $exports[$key];
    },
    array_filter($form_state['values']['exports'])
  );

  switch ($form_state['values']['action']) {

    case 'enable':
      foreach ($selected as $info) {
        $export = apple_news_get_export($info['module'], $info['machine_name']);
        $export->enabled = TRUE;
        apple_news_export_save($export);
      }
      break;

    case 'disable':
      foreach ($selected as $info) {
        $export = apple_news_get_export($info['module'], $info['machine_name']);
        $export->enabled = FALSE;
        apple_news_export_save($export);
      }
      break;

    case 'export':
      foreach ($selected as $info) {
        // Batch entity export to Apple News.
        $batch = apple_news_bactch_run_export($info);
        batch_set($batch);
      }
      break;

  }
}

/**
 * Form builder for an export config.
 *
 * @param AppleNewsExport $export
 *   Export object.
 */
function apple_news_export_edit_form($form, &$form_state, AppleNewsExport $export) {
  $form = $export->getConfigForm($form, $form_state);

  $form['#apple_news_export'] = $export;

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save changes'),
    ),
  );

  $form['#submit'][] = 'apple_news_export_edit_form_submit';
  $form['#theme'] = 'system_settings_form';

  return $form;
}

/**
 * Form validatation handler.
 */
function apple_news_export_edit_form_validate($form, &$form_state) {
  /** @var AppleNewsExport $export */
  $export = $form['#apple_news_export'];
  $export->validateConfigForm($form, $form_state);
}

/**
 * Form submit handler.
 */
function apple_news_export_edit_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  /** @var AppleNewsExport $export */
  $export = $form['#apple_news_export'];
  $export->submitConfigForm($form, $form_state);
  apple_news_export_save($export);
  drupal_set_message(t('Changes have been successfully saved.'));
}
