<?php

/**
 * @file
 * Administrative pages.
 */

/**
 * Form builder.
 */
function apple_news_settings_form($form, &$form_state) {
  $form['apple_news_variable_foo'] = array(
    '#type' => 'textfield',
    '#title' => t('Foo'),
    '#default_value' => variable_get('apple_news_variable_foo', 42),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

/**
 * Form validate handler.
 */
function apple_news_settings_form_validate($form, &$form_state) {
}

/**
 * Form submit handler.
 */
function apple_news_settings_form_submit($form, &$form_state) {

}
