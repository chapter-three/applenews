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
  module_load_include('inc', 'apple_news', 'apple_news.batch');
  $export = apple_news_export_load($module, $machine_name);
  if ($export->enabled) {
    $batch = apple_news_batch_run_export($info, $export);
    $batch['progressive'] = FALSE;
    batch_set($batch);
    drush_log('Entities successfully exported to Apple News.', 'ok');
  }
  else {
    drush_log($info['name'] . ' export is disabled.', 'warning');
  }
}
