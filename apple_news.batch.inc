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
function apple_news_batch_run_export($info, $export, $op) {

  $operations = [];

  switch ($op) {

    case 'export-to-file' :
      $op_function = 'apple_news_batch_preview_generate';
      $op_finish = 'apple_news_batch_export_to_file_finished';
      break;

    case 'export' :
    default :
      $op_function = 'apple_news_batch_post';
      $op_finish = 'apple_news_batch_run_export_finished';
      break;

  }

  $result = $export->query()->execute();
  $entity_types = $export->entityTypes();

  foreach ($entity_types as $entity_type) {
    if (isset($result[$entity_type])) {
      foreach ($result[$entity_type] as $entity_id => $object) {
        // Load entity object and pass it to the document generator.
        $entity = entity_load_single($entity_type, $entity_id);
        $operations[] = [
          $op_function,
          [
            $info,
            $export,
            $entity_type,
            $entity,
            count($result[$entity_type])
          ],
        ];
      }
    }
  }

  return [
    'operations' => $operations,
    'finished' => $op_finish,
  ];
}

/**
 * Batch operation for bactch export.
 *
 * This is the function that is called on each operation in Apple News export.
 */
function apple_news_batch_post($info, $export, $entity_type, $entity, $total_count, &$context) {
  module_load_include('inc', 'apple_news');

  $export_settings = $export->settings;

  // Add data to context so it can be resused in 'finished' callback.
  if (!isset($context['results']['info']['starttime'])) {
    $context['results']['info']['starttime'] = REQUEST_TIME;
  }
  $context['results']['info']['name'] = $info['name'];
  $context['results']['info']['module'] = $info['module'];
  $context['results']['info']['machine_name'] = $info['machine_name'];
  $context['results']['info']['default_channels'] = !empty($export_settings['channels'])
    ? array_keys($export_settings['channels']) : [];

  if (!isset($context['results']['info']['count'])) {
    $context['results']['info']['count'] = 0;
  }

  if (!isset($context['results']['info']['step_count'])) {
    $context['results']['info']['step_count'] = 0;
  }

  $count = !empty($context['results']['info']['step_count']) ? $context['results']['info']['step_count'] : 0;

  // Save batch progress status. Export status indicator visible in the Status column.
  $status = [
    'count'     => $count,
    'starttime' => !empty($context['results']['info']['starttime']) ? $context['results']['info']['starttime'] : 0,
    'status'    => 'in progress ' . $count . '/' . $total_count,
  ];
  $export_id = apple_news_export_id($info['module'], $info['machine_name']);
  apple_news_batch_export_set_status($export_id, $status);

  // Successull request.
  $success = FALSE;

  $wrapper = entity_metadata_wrapper($entity_type, $entity);

  // Check default channel settings for each export.
  // This is a requried field, make sure to enable at least one channel.
  $settings = [];
  if (isset($export_settings['sections'])) {
    foreach ($export_settings['sections'] as $section_id => $checked) {
      if ($checked) {
        $settings['metadata']['data']['links']['sections'][] = apple_news_section_url($section_id);
      }
    }
  }

  if (!empty($export_settings['channels'])) {
    foreach ($export_settings['channels'] as $channel_id => $checked) {
      if (!$checked) {
        continue;
      }
      $channel = apple_news_channel($channel_id);
      $context['results'][] = $wrapper->getIdentifier() . ' : ' . check_plain($wrapper->label());
      $context['message'] = t('<strong>@export</strong>: Posting "@title" entity to <em>@channel</em> channel.',
        [
          '@export'  => $info['name'],
          '@title'   => $wrapper->label(),
          '@channel' => filter_xss($channel->name)
        ]
      );
      // Check if the article had already been exported.
      $export_log = apple_news_is_exported($channel_id, $wrapper, $entity_type);
      // Process only non-published articles.
      if (!$export_log) {
        // Post an article to Apple News.
        $success = apple_news_op('insert', $channel_id, $entity, $entity_type, $export, $settings);
      }
      else {
        // Update an article only if the entity was updated since the last export.
        if ($export_log['postdate'] < $entity->changed) {
          $success = apple_news_op('update', $channel_id, $entity, $entity_type, $export, $settings);
        }
      }
      if ($success) {
        $context['results']['info']['count']++;
      }
    }
    $context['results']['info']['step_count']++;
  }

}

/**
 * Batch 'finished' callback used by batch Apple News export.
 */
function apple_news_batch_run_export_finished($success, $results, $operations) {
  module_load_include('inc', 'apple_news');
  if ($success) {
    if (empty($results['info']['default_channels'])) {
      drupal_set_message(
        t('Please configure default channel settings for <strong>@export_name</strong> export.',
          [
            '@export_name' => !empty($results['info']['name']) ? $results['info']['name'] : ''
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
          'module'       => $results['info']['module'],
          'machine_name' => $results['info']['machine_name'],
          'starttime'    => $results['info']['starttime'],
          'endtime'      => REQUEST_TIME,
          'numprocessed' => $count,
        ];
        drupal_write_record('apple_news_log', $record);

        // Save batch progress status.
        $status = [
          'count'     => $count,
          'starttime' => !empty($results['info']['starttime']) ? $results['info']['starttime'] : 0,
          'endtime'   => REQUEST_TIME,
          'status'    => 'complete',
        ];
        $export_id = apple_news_export_id($results['info']['module'], $results['info']['machine_name']);
        apple_news_batch_export_set_status($export_id, $status);

        $plural_message = format_plural(
          $count,
          '1 entity successfully posted to Apple News.',
          '@count entities were successfully posted to Apple News.'
        );
        drupal_set_message($plural_message);
        $msg = t('Batch export: %count entities successfully posted to Apple News.');
        $args = [
          '%count'   => $count,
        ];
        watchdog('apple_news', $msg, $args, WATCHDOG_NOTICE);
      }
      else {

        // Save batch progress status.
        $status = [
          'count'     => 0,
          'starttime' => !empty($results['info']['starttime']) ? $results['info']['starttime'] : 0,
          'endtime'   => REQUEST_TIME,
          'status'    => 'complete',
        ];
        $export_id = apple_news_export_id($results['info']['module'], $results['info']['machine_name']);
        apple_news_batch_export_set_status($export_id, $status);

        drupal_set_message(t('There is no content for export to Apple News.'));
      }
    }
  }
  else {
    // Save batch progress status.
    $status = [
      'count'     => !empty($results['info']['count']) ? $results['info']['count'] : 0,
      'starttime' => !empty($results['info']['starttime']) ? $results['info']['starttime'] : 0,
      'endtime'   => REQUEST_TIME,
      'status'    => 'error occured',
    ];
    $export_id = apple_news_export_id($results['info']['module'], $results['info']['machine_name']);
    apple_news_batch_export_set_status($export_id, $status);
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    drupal_set_message(
      t('An error occurred while processing @operation with arguments : @args',
        [
          '@operation' => $error_operation[0],
          '@args'      => print_r($error_operation[0], TRUE),
        ]
      )
    );
  }
}

/**
 * Batch operation for bactch export.
 *
 * This is the function that is called on each operation in Export to File.
 */
function apple_news_batch_preview_generate($info, $export, $entity_type, $entity, $total_count, &$context) {
  module_load_include('inc', 'apple_news');

  if (!isset($context['results']['count'])) {
    $context['results']['count'] = 0;
  }

  $entity_wrapper = entity_metadata_wrapper($entity_type, $entity);
  $context['results'][] = $entity_wrapper->getIdentifier() . ' : ' . check_plain($entity_wrapper->label());
  $context['message'] = t('<strong>@export</strong>: Exporting "@title" entity to a file.',
    [
      '@export' => $info['name'],
      '@title'  => $entity_wrapper->label(),
    ]
  );

  $document = $export->export($entity);
  $data = [
    'json'  => $document->json(),
    'files' => $export->getAssets(),
  ];

  $context['results']['info'] = $info;
  $context['results']['entity_ids'][] = $entity_wrapper->getIdentifier();

  // Generate article files for an entity.
  apple_news_export_to_file($entity_wrapper->getIdentifier(), [], NULL, FALSE, $data);

  $context['results']['count']++;

}

/**
 * Batch 'finished' callback used by batch Apple News export to file.
 */
function apple_news_batch_export_to_file_finished($success, $results, $operations) {
  module_load_include('inc', 'apple_news');
  $count = isset($results['count']) ? $results['count'] : 0;
  if ($success) {
    if ($count > 0) {
      // Archive exported artciles and generate a link to the archive.
      $file_url = apple_news_export_to_file(NULL, $results['entity_ids'], $results['info']['module'] . '-' . $results['info']['machine_name'], FALSE, []);
      // Remove exported entity directories.
      apple_news_export_dir_cleanup($results['entity_ids']);
      // Show message.
      if (!empty($file_url)) {
        $download = t('<strong><a href="!link">Download</a></strong> the file and preview articles in the Apple News Preview Tool.',
          [
            '!link' => $file_url,
          ]
        );
        $plural_message = format_plural(
          $count,
          '1 entity successfully exported to a file. ' . $download,
          '@count entities were successfully exported to a file. ' . $download
        );
        drupal_set_message($plural_message);
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
          '@args'      => print_r($error_operation[0], TRUE),
        ]
      )
    );
  }
}
