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
function apple_news_batch_run_export($info, $export) {

  $operations = [];

  $result = $export->query()->execute();
  $entity_types = $export->entityTypes();

  foreach ($entity_types as $entity_type) {
    foreach ($result[$entity_type] as $entity_id => $object) {
      // Load entity object and pass it to the document generator.
      $entity = entity_load_single($entity_type, $entity_id);
      $operations[] = [
        'apple_news_batch_post',
        [
          $info,
          $export,
          $entity_type,
          $entity
        ],
      ];
    }
  }

  return [
    'operations' => $operations,
    'finished' => 'apple_news_batch_run_export_finished',
  ];
}

/**
 * Batch operation for bactch export.
 *
 * This is the function that is called on each operation in Apple News export.
 */
function apple_news_batch_post($info, $export, $entity_type, $entity, &$context) {
  module_load_include('inc', 'apple_news');

  $export_settings = $export->settings;

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

  // Successull request.
  $success = FALSE;

  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  // Check default channel settings for each export.
  // This is a requried field, make sure to enable at least one channel.
  if (!empty($export_settings['channels'])) {
    foreach ($export_settings['channels'] as $channel_id => $checked) {
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
function apple_news_batch_run_export_finished($success, $results, $operations) {
  if ($success) {
    if (empty($results['info']['default_channels'])) {
      drupal_set_message(
        t('Please configure default Feed settings for <strong>@export_name</strong> export.',
          [
            '@export_name' => $results['info']['name']
          ]
        ),
        'warning'
      );
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

/**
 * Batch Apple News export to file.
 *
 * @see apple_news_settings_exports_form_submit()
 */
function apple_news_batch_export_to_file($info, $export) {

  $operations = [];

  $result = $export->query()->execute();
  $entity_types = $export->entityTypes();

  foreach ($entity_types as $entity_type) {
    foreach ($result[$entity_type] as $entity_id => $object) {
      // Load entity object and pass it to the document generator.
      $entity = entity_load_single($entity_type, $entity_id);
      $operations[] = [
        'apple_news_batch_preview_generate',
        [
          $export,
          $entity_type,
          $entity
        ],
      ];
    }
  }

  return [
    'operations' => $operations,
    'finished' => 'apple_news_batch_export_to_file_finished',
  ];
}

/**
 * Batch operation for bactch export.
 *
 * This is the function that is called on each operation in Export to File.
 */
function apple_news_batch_preview_generate($export, $entity_type, $entity, &$context) {
  module_load_include('inc', 'apple_news');

  $entity_wrapper = entity_metadata_wrapper($entity_type, $entity);
  $context['results'][] = $entity_wrapper->getIdentifier() . ' : ' . check_plain($entity_wrapper->label());
  $context['message'] = t('Exporting "@title" entity to a file.',
    [
      '@title' => $entity_wrapper->label(),
    ]
  );

  $document = $export->export($entity);
  $data = [
    'json' => $document->json(),
    'files' => $export->assets($entity),
  ];

  module_load_include('inc', 'apple_news', 'include/preview');
  $preview = new ChapterThree\AppleNews\Preview\ApplenewsPreview();
  $preview->init($entity_wrapper->getIdentifier(), $data);
  $preview->toFile();
  $context['results']['preview'] = $preview;

}

/**
 * Batch 'finished' callback used by batch Apple News export to file.
 */
function apple_news_batch_export_to_file_finished($success, $results, $operations) {
  module_load_include('inc', 'apple_news', 'include/preview');
  $count = count($results);
  $preview = new ChapterThree\AppleNews\Preview\ApplenewsPreview();
  $preview->init(NULL, []);
  $file_url = $preview->getArchiveFilePath();
  if ($success) {
    if ($count > 0) {
      $download = t('<strong><a href="!link">Download</a></strong> the file and preview articles in Apple News Preview Tool.',
        [
          '!link' => $file_url,
        ]
      );
      $plural_message = format_plural(
        $count,
        '1 entity successfully exported to a file. ' . $download,
        '@count entities were successfully exported to a file. ' . $download
      );
      $preview->archive();
      drupal_set_message($plural_message);
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
