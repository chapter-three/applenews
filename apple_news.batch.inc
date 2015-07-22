<?php
/**
 * @file
 * Batch operations.
 */

/**
 * Batch Apple News export.
 */
function apple_news_bactch_run_export($info, $export) {

  $operations = array();

  $result = $export->query()->execute();
  $entity_types = array_keys($result);

  foreach ($entity_types as $entity_type) {
    foreach ($result[$entity_type] as $entity_id => $object) {
      // Load entity object and pass it to the document generator.
      $entity = entity_load_single($entity_type, $entity_id);
      // Generate AppleNews Document formatted JSON.
      $document = $export->export($entity);
      if (!is_null($document)) {
        $operations[] = array(
          'apple_news_bactch_post',
          [
            $info,
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
function apple_news_bactch_post($info, $entity_type, $entity, $document, &$context) {
  module_load_include('inc', 'apple_news', 'push_api');
  $channels = variable_get('apple_news_variable_api_channels', array());

  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  // Add data to context so it can be resused in 'finished' callback.
  $context['results']['info']['starttime'] = REQUEST_TIME;
  $context['results']['info']['module'] = $info['module'];
  $context['results']['info']['machine_name'] = $info['machine_name'];
  if (!isset($context['results']['info']['count'])) {
    $context['results']['info']['count'] = 0;
  }
  if (!isset($context['results']['info']['initialhighwater'])) {
    $context['results']['info']['initialhighwater'] = $wrapper->getIdentifier();
  }
  $context['results']['info']['finalhighwater'] = $wrapper->getIdentifier();

  // Get channels.
  $channels = variable_get('apple_news_variable_api_channels', array());
  foreach ($channels as $channel_id => $channel_info) {
    $context['results'][] = $wrapper->getIdentifier() . ' : ' . check_plain($wrapper->label());
    $context['message'] = t('Posting "@title" entity to <em>@channel</em> feed.', [
        '@title' => $wrapper->label(),
        '@channel' => $channels[$channel_id]['name']
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
      $context['results']['info']['count']++;
    }
  }

}

/**
 * Batch 'finished' callback used by batch Apple News export.
 */
function apple_news_bactch_run_export_finished($success, $results, $operations) {
  if ($success) {
  	$count = $results['info']['count'];
  	// Save export information in logs.
    $record = [
      'module' => $results['info']['module'],
      'machine_name' => $results['info']['machine_name'],
      'starttime' => $results['info']['starttime'],
      'endtime' => REQUEST_TIME,
      'initialhighwater' => $results['info']['initialhighwater'],
      'finalhighwater' => $results['info']['finalhighwater'],
      'numprocessed' => $count,
    ];
    drupal_write_record('apple_news_log', $record);
    drupal_set_message(t('@count entities were posted to Apple News.', array('@count' => $count)));
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
