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

  foreach ($entity_types as $type) {
    foreach ($result[$type] as $entity_id => $object) {
      // Load entity object and pass it to the document generator.
      $entity = entity_load_single($type, $entity_id);
      $operations[] = array(
        'apple_news_bactch_post',
        [
          $entity,
          '',
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
function apple_news_bactch_post($entity, $operation_details, &$context) {

  // Generate AppleNews Document formatted JSON.
  $document = $export->export($entity);

  // Contents of the article.json file.
  $json = $document->__toString();

  // Get channel IDs.
  $channels = apple_news_channels();
  foreach ($channels as $channel_id) {
    $context['results'][] = $entity_id . ' : ' . check_plain($entity->label());
    $context['message'] = t('Posting "@title" to Apple News\'s channel @channel.', [
        '@title' => $entity->label(),
        '@channel' => $channel_id
      ]
    );
    apple_news_post_article($channel_id, [
        'json' => $json,
        'files' => [],
        'metadata' => '',
      ]
    );
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
