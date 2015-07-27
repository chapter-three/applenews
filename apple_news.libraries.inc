<?php
/**
 * @file
 * Drush hook implementations.
 */

/**
 * Implements hook_libraries_info().
 *
 * @todo Test implementation.
 */
function apple_news_libraries_info() {

  $libraries['AppleNews'] = [
    'name' => t('Apple News Document and PushAPI PHP library'),
    'download url' => 'https://github.com/chapter-three/AppleNews',
    'files' =>
    [
      'php' =>
      [
        'src/Document.php',
        'src/PushAPI.php',
      ],
    ],
  ];

  $libraries['php-curl-class'] = [
    'name' => t('PHP Curl Class'),
    'download url' => 'https://github.com/php-curl-class/php-curl-class',
    'files' =>
    [
      'php' =>
      [
        'src/Curl/Curl.php',
      ],
    ],
  ];

  return $libraries;

}
