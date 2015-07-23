<?php
/**
 * @file
 * Drush hook implementations.
 */

/**
 * Implements hook_drush_command().
 */
function apple_news_drush_command() {
  $items['apple-news-export'] = [
    'description' => 'Export entities to Apple News.',
    'arguments' => [
      'module' => 'Apple News Export module name',
      'machine_name' => 'Apple News Export machine name',
    ],
    'examples' => [
      'Run [module] [machine_name] export.' => 'drush apple-news-export [module] [machine_name]',
      'Run [module] [machine_name] export. (alias example)' => 'drush ane [module] [machine_name]',
    ],
    'aliases' => ['ane'],
  ];
  return $items;
}

/**
 * Callback for the "apple-news-export" command.
 */
function drush_apple_news_apple_news_export($module, $machine_name) {
  module_load_include('inc', 'apple_news');
  drush_log('Greenhouse jobs cache renewed', 'ok');
}
