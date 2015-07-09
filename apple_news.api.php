<?php

/**
 * @file
 * Hooks provided by the Apple News module.
 */

/**
 * Registers your Apple News classes and defines defaults.
 *
 * @return array
 *   An associative array with the following keys:
 *   - api (required): Always 1 for any module implementing the Apple News 1
 *     API.
 *   - exports: Associative array of exports defined by this module, keyed on
 *     export machine name, with keys:
 *     -  class: Class name to instantiate for the export.
 *     -  arguments: Array of arguments to pass to class constructor.
 *
 * @see hook_apple_news_api_alter()
 */
function hook_apple_news_api() {
  return array(
    'api' => 1,
    'exports' => array(
      'apple_news_article' => array(
        'class' => 'AppleNewsExportArticle',
        'arguments' => array(),
      ),
    ),
  );
}

/**
 * Alter information from all implementations of hook_apple_news_api().
 *
 * @param array $info
 *   An array of results from hook_apple_news_api(), keyed by module name.
 *
 * @see hook_apple_news_api()
 */
function hook_apple_news_api_alter(array &$info) {
  // Override the class for another module's migration - say, to add some
  // additional preprocessing in prepareRow().
  if (isset($info['MODULE_NAME']['key'])) {
    $info['MODULE_NAME']['key'] = 'new value';
  }
}
