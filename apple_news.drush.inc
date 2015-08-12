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
function drush_apple_news($op, $module, $machine_name) {

  module_load_include('inc', 'apple_news');
  module_load_include('inc', 'apple_news', 'apple_news.batch');

  $export = apple_news_export_load($module, $machine_name);

  if ($export->enabled) {
    if ($op == 'export') {
      // Run brach enttiy export to Apple News.
      $batch = apple_news_batch_run_export($info, $export);
      $batch['progressive'] = FALSE;
      batch_set($batch);
      drush_log('Entities successfully exported to Apple News.', 'ok');
    }
    elseif ($op == 'preview') {
      // Download News Preview compatible files.
      $batch = apple_news_batch_export_to_file($info, $export);
      $batch['progressive'] = FALSE;
      batch_set($batch);
      drush_log('Apple News Native formatted entities successfully exported. The file archive file can be downloaded from files/apple-news/ directory.', 'ok');
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

  $libraries = [
    'AppleNews'      => APPLE_NEWS_SUPPORTED_VERSION,
    'php-curl-class' => CURL_SUPPORTED_VERSION,
  ];

  foreach ($libraries as $name => $version) {
    if ($library = libraries_load($name)) {
      $lib_path = $library['library path'];
      $lib_all_realpath = DRUPAL_ROOT . '/sites/all/libraries';
      $lib_realpath = DRUPAL_ROOT . '/' . $lib_path;
      $lib_version_realpath = $lib_realpath . '-' . $version;
      $download_url = $library['download url'];
      if (is_dir($lib_realpath)) {
        continue;
      }
      // Get remote library file.
      $request = drupal_http_request($download_url);
      // If file exists.
      if ($request->code == 200) {
        if (!is_dir($lib_version_realpath)) {
          // Get archive basename.
          $filename = drupal_basename($download_url);
          $filepath = $lib_all_realpath . '/' . $filename;
          // Save remote archive file locally.
          file_unmanaged_save_data($request->data, $filepath);
          // Unzip the file.
          $zip = new \ZipArchive();
          if ($zip->open($filepath) === TRUE) {
            // Extract library archive file.
            $zip->extractTo($lib_all_realpath);
            // Delete the archive file.
            file_unmanaged_delete_recursive($lib_all_realpath . '/' . $filename);
            // Rename unzipped directory to match the lib path.
            @rename($lib_version_realpath, $lib_realpath);
            drush_log($library['name'] . ' successfully downloaded and unzipped.', 'ok');
          } 
          else {
            drush_log('Failed to unzip ' . $library['name'] . '.', 'warning');
          }
          $zip->close();
        }
      }
    }
  }

}
