<?php

/**
 * @file
 * Administrative pages.
 */

/**
 * Form builder for API config.
 */
function apple_news_settings_form($form, &$form_state) {

  $form['apple_news_variable_api_endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('API Endpoint'),
    '#default_value' => variable_get('apple_news_variable_api_endpoint', ''),
  );

  $form['apple_news_variable_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#default_value' => variable_get('apple_news_variable_api_key', ''),
  );

  $form['apple_news_variable_api_secret'] = array(
    '#type' => 'password',
    '#title' => t('API Secret'),
    '#default_value' => variable_get('apple_news_variable_api_secret', ''),
  );

  $form['apple_news_variable_api_channel'] = array(
    '#type' => 'textfield',
    '#title' => t('API Channel'),
    '#default_value' => variable_get('apple_news_variable_api_channel', ''),
  );

  $form = system_settings_form($form);
  $form['#submit'][] = 'apple_news_settings_form_submit';
  return $form;
}

/**
 * Form submit handler.
 */
function apple_news_settings_form_submit($form, &$form_state) {
  if ($response = apple_news_get_channel()) {
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

/**
 * Form builder for node export config.
 */
function apple_news_settings_node_form($form, &$form_state) {

  $form['apple_news_variable_node'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable'),
    '#description' => t('Enable node export. Export configuration can be found on the !link.', array('!link' => l(t('content type configuration page'), 'admin/structure/types'))),
    '#default_value' => variable_get('apple_news_variable_node', FALSE),
  );

  $form = system_settings_form($form);
  return $form;
}
