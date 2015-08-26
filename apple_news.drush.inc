<?php
/**
 * @file
 * Drush hook implementations.
 */

/**
 * Implements hook_drush_command().
 */
function apple_news_drush_command() {

  $items['apple-news'] = [
    'description' => 'Export entities to Apple News.',
    'arguments' => [
      'op'           => 'Operation export/preview/install',
      'module'       => 'Apple News Export module name',
      'machine_name' => 'Apple News Export machine name',
    ],
    'examples' => [
      'Run [module] [machine_name] export.'
        => 'drush apple-news export [module] [machine_name]',
      'Run [module] [machine_name] export to file.'
        => 'drush apple-news preview [module] [machine_name]',
    ],
    'aliases' => ['an'],
  ];

  $items['apple-news-libraries'] = [
    'description' => 'Download Apple News dependency PHP libraries.',
    'examples' => [
      'Download and unarchive libraries.'
        => 'drush apple-news-libraries',
    ],
    'aliases' => ['an-libs'],
  ];

  return $items;

}

/**
 * Callback for the "apple-news" command.
 */
function drush_apple_news($operation, $module, $machine_name) {

  module_load_include('inc', 'apple_news');
  module_load_include('inc', 'apple_news', 'apple_news.batch');

  $export = apple_news_export_load($module, $machine_name);

  if ($export->enabled) {

    switch ($operation) {

      // Run brach enttiy export to Apple News.
      case 'export' :
        $batch = apple_news_batch_run_export($info, $export, 'export');
        $batch['progressive'] = FALSE;
        batch_set($batch);
        drush_log('Entities successfully exported to Apple News.', 'ok');
        break;

      // Download News Preview compatible files.
      case 'preview' :
        $batch = apple_news_batch_run_export($info, $export, 'export-to-file');
        $batch['progressive'] = FALSE;
        batch_set($batch);
        drush_log('Apple News Native formatted entities successfully exported. The file archive file can be downloaded from `files/apple-news/` directory.', 'ok');
        break;

    }

  }
  else {
    drush_log($info['name'] . ' export is disabled.', 'warning');
  }

}

/**
 * Callback for the "apple-news-libraries" command.
 * Downloads remote ZIP archive, extracts the file and
 * create library specific directory.
 */
function drush_apple_news_libraries() {
  module_load_include('inc', 'apple_news');
  // Download and unzip PHP libraries.
  // @see apple_news.inc file.
  apple_news_autoinstall_libraries();
}
