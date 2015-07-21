<?php
/**
 * @file
 * Batch operations.
 */

/**
 * Batch Apple News export.
 */
function apple_news_bactch_run_export($info) {

  $operations = array();

  $export = apple_news_get_export($info['module'], $info['machine_name']);
  $result = $export->query()->execute();

  $entity_types = array_keys($result);

  foreach ($entity_types as $entity_type) {
    foreach ($result[$entity_type] as $entity_id => $object) {
      // Load entity object and pass it to the document generator.
      $entity = entity_load_single($entity_type, $entity_id);

      // Generate AppleNews Document formatted JSON.
      $document = $export->export($entity);
      $operations[] = array(
        'apple_news_bactch_post',
        [
          $entity_type,
          $entity,
          [
            'json' => $document->__toString(),
            'files' => [],
            'metadata' => '',
          ],
        ],
      );
    }
  }

  return [
    'operations' => $operations,
    'finished' => 'apple_news_bactch_run_export_finished',
  ];
}

/**
 * Batch operation for bactch export.
 *
 * This is the function that is called on each operation in batch 1.
 */
function apple_news_bactch_post($entity_type, $entity, $document, &$context) {
  module_load_include('inc', 'apple_news', 'push_api');

  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  // Get channels.
  $channels = variable_get('apple_news_variable_api_channels', array());
  foreach ($channels as $channel_id => $channel_info) {
    $context['results'][] = $wrapper->getIdentifier() . ' : ' . check_plain($wrapper->label());
    $context['message'] = t('Posting "@title" to Apple News\'s channel @channel.', [
        '@title' => $wrapper->label(),
        '@channel' => $channel_id
      ]
    );
    // Make POST request to PushAPI.
    $response = apple_news_post_article($channel_id, $document);
    // If sucessfull request.
    if (!isset($response->errors)) {
      $record = [
        'entity_type' => $entity_type,
        'entity_id' => $wrapper->getIdentifier(),
        'article_id' => $response->data->id,
        'channel_id' => $channel_id,
        'postdate' => REQUEST_TIME,
      ];
      drupal_write_record('apple_news_published_entities', $record);
    }
  }

}

/**
 * Batch 'finished' callback used by batch Apple News export.
 */
function apple_news_bactch_run_export_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('@count entities were posted to Apple News.', array('@count' => count($results))));
    drupal_set_message(t('The final result was "%final"', array('%final' => end($results))));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    drupal_set_message(
      t('An error occurred while processing @operation with arguments : @args',
        array(
          '@operation' => $error_operation[0],
          '@args' => print_r($error_operation[0], TRUE),
        )
      )
    );
  }
}
