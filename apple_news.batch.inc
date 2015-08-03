<?php
/**
 * @file
 * Batch operations.
 */

/**
 * Batch Apple News export.
 *
 * @see apple_news_settings_exports_form_submit()
 */
function apple_news_bactch_run_export($info, $export) {

  $operations = [];

  $result = $export->query()->execute();
  $entity_types = $export->entityTypes();

  foreach ($entity_types as $entity_type) {
    foreach ($result[$entity_type] as $entity_id => $object) {
      // Load entity object and pass it to the document generator.
      $entity = entity_load_single($entity_type, $entity_id);
      // Generate AppleNews Document.
      $document = $export->export($entity);
      if (!is_null($document)) {
        $operations[] = [
          'apple_news_bactch_post',
          [
            $info,
            $export->settings,
            $entity_type,
            $entity,
            $export
          ],
        ];
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
function apple_news_bactch_post($info, $export_settings, $entity_type, $entity, $export, &$context) {
  module_load_include('inc', 'apple_news');

  // Successull request.
  $success = FALSE;

  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  // Add data to context so it can be resused in 'finished' callback.
  $context['results']['info']['starttime'] = REQUEST_TIME;
  $context['results']['info']['name'] = $info['name'];
  $context['results']['info']['module'] = $info['module'];
  $context['results']['info']['machine_name'] = $info['machine_name'];
  $context['results']['info']['default_channels'] = !empty($export_settings['channels']) 
    ? array_keys($export_settings['channels']) : [];
  if (!isset($context['results']['info']['count'])) {
    $context['results']['info']['count'] = 0;
  }

  // Check default channel settings for each export.
  // This is a requried field, make sure to enable at least one channel.
  if (!empty($export_settings['channels'])) {
    foreach (array_keys($export_settings['channels']) as $channel_id) {
      $channel = apple_news_channel($channel_id);
      $context['results'][] = $wrapper->getIdentifier() . ' : ' . check_plain($wrapper->label());
      $context['message'] = t('Posting "@title" entity to <em>@channel</em> feed.',
        [
          '@title' => $wrapper->label(),
          '@channel' => filter_xss($channel->name)
        ]
      );
      // Check if the article had already been exported.
      $export_log = apple_news_is_exported($channel_id, $wrapper, $entity_type);
      // Process only non-published articles.
      if (!$export_log) {
        // Post an article to Apple News.
        $success = apple_news_op('insert', $channel_id, $entity, $entity_type, $export);
      }
      else {
        // Update an article only if the entity was updated since the last export.
        if ($export_log->postdate < $entity->changed) {
          $success = apple_news_op('update', $channel_id, $entity, $entity_type, $export);
        }
      }
      if ($success) {
        $context['results']['info']['count']++;
      }
    }
  }

}

/**
 * Batch 'finished' callback used by batch Apple News export.
 */
function apple_news_bactch_run_export_finished($success, $results, $operations) {
  if ($success) {
    if (empty($results['info']['default_channels'])) {
      drupal_set_message(
        t('Please configure default Feed settings for <strong>@export_name</strong> export.', ['@export_name' => $results['info']['name']]), 'warning');
    }
    else {
      $count = $results['info']['count'];
      if ($count > 0) {
        // Save export information in logs.
        $record = [
          'module' => $results['info']['module'],
          'machine_name' => $results['info']['machine_name'],
          'starttime' => $results['info']['starttime'],
          'endtime' => REQUEST_TIME,
          'numprocessed' => $count,
        ];
        drupal_write_record('apple_news_log', $record);
        $plural_message = format_plural(
          $count,
          '1 entity successfully posted to Apple News.',
          '@count entities were successfully posted to Apple News.'
        );
        drupal_set_message($plural_message);
      }
      else {
        drupal_set_message(t('There is no content for export to Apple News.'));
      }
    }
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    drupal_set_message(
      t('An error occurred while processing @operation with arguments : @args',
        [
          '@operation' => $error_operation[0],
          '@args' => print_r($error_operation[0], TRUE),
        ]
      )
    );
  }
}
