<?php

/**
 * @file
 * Administrative pages.
 */

/**
 * Form builder for API config.
 */
function apple_news_settings_api_credentials_form($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  $form['credentials'] = [
    '#type' => 'fieldset',
    '#title' => t('Apple News Credentials')
  ];

  $endpoint = variable_get('apple_news_variable_api_endpoint', '');
  $api_key = variable_get('apple_news_variable_api_key', '');
  $api_secret = variable_get('apple_news_variable_api_secret', '');

  $form['credentials']['apple_news_variable_api_endpoint'] = [
    '#type' => 'textfield',
    '#title' => t('API Endpoint URL'),
    '#default_value' => variable_get('apple_news_variable_api_endpoint', ''),
    '#description' => t('Push API endpoint URL'),
  ];

  $form['credentials']['apple_news_variable_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('API Key ID'),
    '#default_value' => $api_key,
    '#description' => t('Push API Key ID'),
  ];

  $form['credentials']['apple_news_variable_api_secret'] = [
    '#type' => 'password',
    '#title' => t('API Secret Key'),
    '#default_value' => $api_secret,
    '#description' => t('Push API Secret Key'),
  ];

  $form['save_config'] = [
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => ['apple_news_settings_api_credentials_form_submit'],
    '#validate' => ['apple_news_settings_api_credentials_form_validate'],
  ];

  // Show delete button only when all the fields are prepopulated.
  if (!empty($endpoint) && !empty($api_key) && !empty($api_secret)) {
    $form['delete_config'] = [
      '#markup' => l(t('Delete configuration'), 'admin/config/content/apple-news/settings/delete'),
    ];
  }

  return $form;
}

/**
 * Form validation handler.
 */
function apple_news_settings_api_credentials_form_validate($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  foreach (apple_news_settings_fields() as $key) {
    if (isset($form['credentials'][$key]['#title'])) {
      $value = trim($form_state['values'][$key]);
      if ($value === '') {
        form_set_error(
          $key,
          t('Missing required value "@title".',
            [
              '@title' => $form['credentials'][$key]['#title']
            ]
          )
        );
      }
    }
  }
}

/**
 * Form submit handler.
 */
function apple_news_settings_api_credentials_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  foreach (apple_news_settings_fields() as $key) {
    if (isset($form['credentials'][$key]['#title'])) {
      $value = $form_state['values'][$key];
      if (is_array($value) && isset($form_state['values']['array_filter'])) {
        $value = array_keys(array_filter($value));
      }
      variable_set($key, $value);
    }
  }
  drupal_set_message(
    t('The configuration options have been saved. Please <a href="!link">add Feed ID</a> to verify connection and start posting content to Apple News.',
      [
        '!link' => url('admin/config/content/apple-news/settings/channels')
      ]
    )
  );
}

/**
 * Delete Apple News configuration w/ confirmation.
 */
function apple_news_settings_delete_configuration_form($form, &$form_state) {
  return confirm_form(
    $form,
    t("Do you really want to delete current Apple News configuration?"),
    'admin/config/content/apple-news/settings',
    t("You will no longer be able to publish content to Apple News. This action cannot be undone."),
    t("Yes, delete configuration")
  );
}

/**
 * Form submit handler.
 */
function apple_news_settings_delete_configuration_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  foreach (apple_news_settings_fields() as $key) {
    variable_set($key, '');
  }
  // Remove data from custom cache table.
  db_truncate('cache_apple_news')->execute();
  // Reset channels.
  variable_set('apple_news_variable_api_channels', []);
  drupal_set_message(t('The configuration options have been deleted.'));
  drupal_goto('admin/config/content/apple-news/settings');
}

/**
 * Form builder for API config.
 */
function apple_news_settings_channels_form() {
  module_load_include('inc', 'apple_news');

  $endpoint = variable_get('apple_news_variable_api_endpoint', '');
  $api_key = variable_get('apple_news_variable_api_key', '');
  $api_secret = variable_get('apple_news_variable_api_secret', '');

  $channels = [];
  $api_channels = variable_get('apple_news_variable_api_channels', []);
  foreach ($api_channels as $channel_id) {
    $channel = apple_news_channel($channel_id);
    $channels[] = [
      'name' => l(filter_xss($channel->name), 'admin/config/content/apple-news/settings/channels/' . $channel->id . '/sections'),
      'id' => $channel->id,
      'operations' => l('Delete', 'admin/config/content/apple-news/settings/channels/' . $channel->id . '/delete')
    ];
  }

  // Show delete button only when all the fields are prepopulated.
  if (!empty($endpoint) && !empty($api_key) && !empty($api_secret)) {

    $form['channels'] = [
      '#markup' => '<br/>' . theme('table',
        [
          'header' => [
            t('Feed Name'), // Channel Name
            t('Feed ID'), // Channel ID
            t('Operations'),
          ],
          'rows' => $channels,
          'empty' => t('Please add at least one feed in order to be able to publish content to Apple News.'),
        ]
      )
    ];

    $form['apple_news_channel_id'] = [
      '#type' => 'textfield',
      '#title' => t('Feed ID'), // Channel ID
      '#description' => t("Provide a Feed ID"),
      '#required' => TRUE,
    ];

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => t('Add new feed'),
    ];

  }
  else {

    $form['requirements'] = [
      '#markup' => t('Please provide Apple News <a href="!link">credentials</a> in order to be able to see and add Feeds.',
        [
          '!link' => url('admin/config/content/apple-news/settings'),
        ]
      ),
    ];

  }

  return $form;
}

/**
 * Form submit handler.
 */
function apple_news_settings_channels_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  $channel_id = $form_state['values']['apple_news_channel_id'];
  $channels = variable_get('apple_news_variable_api_channels', []);

  if ($channel = apple_news_channel($channel_id)) {
    // Save settings.
    if (!isset($channel->errors)) {
      // Get sections.
      apple_news_sections($channel->id);
      // Save new channel ID.
      $channels[] = $channel->id;
      variable_set('apple_news_variable_api_channels', $channels);
      $msg = t('Successfully connected to the feed "@channel".', [
        '@channel' => filter_xss($channel->name),
      ]);
      drupal_set_message($msg, 'status');
    }
    else {
      $msg = t('There was an error communicating with the server. Please make sure you have correct Feed ID and Apple News credentials.');
      drupal_set_message($msg, 'error');
    }
  }
  else {
    $msg = t('There was an error communicating with the server.');
    drupal_set_message($msg, 'error');
  }

  drupal_set_message(t('The configuration options have been saved.'));

}

/**
 * View Apple News channel sections.
 */
function apple_news_settings_channel_sections_form($form, &$form_state, $channel_id) {
  module_load_include('inc', 'apple_news');

  $sections = [];
  $channel_sections = apple_news_sections($channel_id);
  if (!empty($channel_sections)) {
    foreach ($channel_sections as $section) {
      $sections[] = [
        'name' => filter_xss($section->name),
        'id' => $section->id,
        'default' => ($section->isDefault) ? t('Yes') : '',
      ];
    }
  }

  $form['sections'] = [
    '#markup' => theme('table',
      [
        'header' => [
          t('Section Name'),
          t('Section ID'),
          t('Default')
        ],
        'rows' => $sections
      ]
    ),
  ];
  return $form;
}

/**
 * Delete channel ID with confirmation.
 */
function apple_news_settings_delete_channel_form($form, &$form_state, $channel_id) {
  module_load_include('inc', 'apple_news');
  $channel = apple_news_channel($channel_id);
  $form_state['storage'] = ['channel' => $channel];
  return confirm_form(
    $form,
    t("Do you really want to delete this feed?"),
    'admin/config/content/apple-news/settings/channels',
    t("You will no long be able to publish content to <em>@name</em> feed. This action cannot be undone.",
      [
        '@name' => $channel->name
      ]
    ),
    t("Yes, delete")
  );
}

/**
 * Form submit handler.
 */
function apple_news_settings_delete_channel_form_submit($form, &$form_state) {
  $channel = $form_state['storage']['channel'];
  if (empty($channel->id)) {
    drupal_set_message(t('You are trying to remove non-existing feed.'), 'warning');
  }
  else {
    drupal_set_message(
      t('<em>@name</em> feed successfully removed.',
        [
          '@name' => filter_xss($channel->name)
        ]
      )
    );

    $channels = variable_get('apple_news_variable_api_channels', []);
    foreach ($channels as $i => $channel_id) {
      if ($channel_id == $channel->id) {
        unset($channels[$i]);
      }
    }
    variable_set('apple_news_variable_api_channels', $channels);

    // Delete from custom cache table.
    db_delete('cache_apple_news')
      ->condition('cid', 'channel_' . $channel->id)
      ->execute();
    db_delete('cache_apple_news')
      ->condition('cid', 'sections_' . $channel->id)
      ->execute();

    // Delete settings when channel ID removed.
    $result = db_select('apple_news_status', 's')
      ->fields('s')
      ->execute();

    foreach ($result as $row) {
      $settings = unserialize($row->settings);
      unset($settings['channels'][$channel->id]);
      db_update('apple_news_status')
        ->fields(
          [
            'settings' => serialize($settings),
          ]
        )
        ->condition('module', $row->module)
        ->condition('machine_name', $row->machine_name)
        ->execute();
    }
    drupal_goto('admin/config/content/apple-news/settings/channels');
  }
}

/**
 * Form builder for export config.
 */
function apple_news_settings_exports_form($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  $header = [
    'name' => ['data' => t('Name')],
    'description' => ['data' => t('Description')],
    'module' => ['data' => t('Module')],
    'enabled' => ['data' => t('Enabled')],
    'status' => ['data' => t('Status')],
    'lastexport' => ['data' => t('Last Export')],
    'edit' => ['data' => ''],
  ];

  $rows = [];
  foreach (apple_news_exports() as $key => $info) {
    $query = db_select('apple_news_log', 'l')
      ->fields('l', ['starttime', 'endtime'])
      ->condition('l.module', $info['module'])
      ->condition('l.machine_name', $info['machine_name'])
      ->orderBy('l.starttime', 'DESC');
    $log = $query->range(0, 1)->execute()->fetchAssoc();

    $name = empty($info['name']) ? $info['machine_name'] : $info['name'];
    $module = system_get_info('module', $info['module']);
    $module = $module['name'];
    $enabled = empty($info['#status']['enabled']) ? t('no') : t('yes');
    $status = empty($log['endtime']) ? (empty($log['starttime']) ? '' : t('incomplete')) : t('complete');
    $lastexport = empty($log['starttime']) ? t('never') : format_date($log['starttime'], 'short');
    $edit = l(t('edit'),
      'admin/config/content/apple-news/exports/' . $key . '/edit',
      [
        'query' => [
          'destination' => 'admin/config/content/apple-news/exports',
        ],
      ]
    );
    $rows[$key] = [
      'name' => $name,
      'description' => $info['description'],
      'module' => $module,
      'enabled' => $enabled,
      'status' => $status,
      'lastexport' => $lastexport,
      'edit' => $edit,
    ];
  }

  $form['exports'] = [
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#tree' => TRUE,
    '#empty' => t('No exports are defined.'),
  ];

  $form['actions'] = [
    '#type' => 'actions',
    'action' =>
    [
      '#type' => 'select',
      '#title' => t('Action'),
      '#options' =>
      [
        'enable' => t('Enable'),
        'disable' => t('Disable'),
        'export' => t('Export'),
      ],
    ],
    'submit' => [
      '#type' => 'submit',
      '#value' => t('Submit'),
    ],
  ];

  $form['#submit'][] = 'apple_news_settings_exports_form_submit';
  $form['#theme'] = 'system_settings_form';

  return $form;
}

/**
 * Form validatation handler.
 */
function apple_news_settings_exports_form_validate($form, &$form_state) {

  $selected = array_filter($form_state['values']['exports']);
  if (empty($selected)) {
    form_set_error('exports', 'Select at lease one export.');
  }

}

/**
 * Form submit handler.
 */
function apple_news_settings_exports_form_submit($form, &$form_state) {

  module_load_include('inc', 'apple_news', 'push_api');
  module_load_include('inc', 'apple_news');

  $exports = apple_news_exports();
  $selected = array_map(function($key) use(&$exports) {
    return $exports[$key];
  }, array_filter($form_state['values']['exports']));

  switch ($form_state['values']['action']) {

    case 'enable':
      foreach ($selected as $info) {
        $export = apple_news_get_export($info['module'], $info['machine_name']);
        $export->enabled = TRUE;
        apple_news_export_save($export);
        drupal_set_message(
          t('<em>@name</em> export sucessfully <strong>enabled</strong>.',
            [
              '@name' => $info['name']
            ]
          )
        );
      }
      break;

    case 'disable':
      foreach ($selected as $info) {
        $export = apple_news_get_export($info['module'], $info['machine_name']);
        $export->enabled = FALSE;
        apple_news_export_save($export);
        drupal_set_message(
          t('<em>@name</em> export sucessfully <strong>disabled</strong>.',
            [
              '@name' => $info['name']
            ]
          )
        );
      }
      break;

    case 'export':
      foreach ($selected as $info) {
        // Batch entity export to Apple News.
        $export = apple_news_get_export($info['module'], $info['machine_name']);
        if ($export->enabled) {
          $batch = apple_news_bactch_run_export($info, $export);
          batch_set($batch);
        }
        else {
          drupal_set_message(
            t('<em>@name</em> export is disabled.',
              [
                '@name' => $info['name']
              ]
            ),
            'warning'
          );
        }
      }
      break;

  }
}

/**
 * Form builder for an export config.
 *
 * @param ApplenewsExport $export
 *   Export object.
 */
function apple_news_export_edit_form($form, &$form_state, ApplenewsExport $export) {
  $form = $export->getConfigForm($form, $form_state);

  $form['#apple_news_export'] = $export;

  $form['actions'] = [
    '#type' => 'actions',
    'submit' => [
      '#type' => 'submit',
      '#value' => t('Save changes'),
    ],
    'cancel' => [
      '#markup' => l(t('Cancel'), 'admin/config/content/apple-news'),
    ],
  ];

  return $form;
}

/**
 * Theme callback for the apple_news_export_edit_form().
 *
 * @see theme_tabledrag_example_parent_form()
 */
function theme_apple_news_export_edit_form($variables) {
  $form = $variables['form'];
  $output = '';

  // In case we want to add more sets of destinations.
  foreach (['components'] as $key) {
    if (empty($form[$key])) {
      continue;
    }

    $tree = &$form[$key];
    $css_prefix = "apple-news-${key}-";

    $rows = [];
    foreach (element_children($tree) as $id) {
      $tree[$id]['weight']['#attributes']['class'] = [$css_prefix . 'weight'];
      $tree[$id]['id']['#attributes']['class'] = [$css_prefix . 'id'];
      $tree[$id]['pid']['#attributes']['class'] = [$css_prefix . 'pid'];
      $class = ['draggable'];
      if (@$tree[$id]['#apple_news_destination']) {
        /** @var ApplenewsDestination $destination */
        $destination = $tree[$id]['#apple_news_destination'];
        if ($destination->isRoot()) {
          $class[] = 'tabledrag-root';
        }
        if ($destination->isLeaf()) {
          $class[] = 'tabledrag-leaf';
        }
      }
      $indent = theme('indentation', ['size' => $tree[$id]['depth']['#value']]);
      unset($tree[$id]['depth']);
      $rows[] = [
        'data' => [
          $indent . drupal_render($tree[$id]['destination']),
          drupal_render($tree[$id]['edit']),
          drupal_render($tree[$id]['delete']),
          drupal_render($tree[$id]['weight']) .
            drupal_render($tree[$id]['id']) .
            drupal_render($tree[$id]['pid']),
        ],
        'class' => $class,
      ];
    }

    $header = [
      t('Component'),
      [
        'data' => t('Operations'),
        'colspan' => 2,
      ],
      t('Weight'),
    ];
    $table_id = $css_prefix . 'table';
    $output .= theme('table', [
      'header' => $header,
      'rows' => $rows,
      'attributes' => ['id' => $table_id],
    ]);
    drupal_add_tabledrag($table_id, 'match', 'parent',
      $css_prefix . 'pid',
      $css_prefix . 'pid',
      $css_prefix . 'id',
      TRUE);
    drupal_add_tabledrag($table_id, 'order', 'sibling',
      $css_prefix . 'weight',
      NULL,
      NULL,
      TRUE);
  }

  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Form validatation handler.
 */
function apple_news_export_edit_form_validate($form, &$form_state) {
  /** @var ApplenewsExport $export */
  $export = $form['#apple_news_export'];
  $export->validateConfigForm($form, $form_state);
}

/**
 * Form submit handler.
 */
function apple_news_export_edit_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  /** @var ApplenewsExport $export */
  $export = $form['#apple_news_export'];
  $export->submitConfigForm($form, $form_state);
  apple_news_export_save($export);
  drupal_set_message(t('Changes have been successfully saved.'));
}

/**
 * Form builder for an export config.
 *
 * @param ApplenewsExportConfigurable $export
 *   Export object.
 */
function apple_news_export_destination_edit_form($form, &$form_state, ApplenewsExportConfigurable $export, $type, $id) {

  switch ($type) {

    case 'components':
      /** @var ApplenewsDestination $destination */
      $destination = $export->getComponent($id);
      $form = $destination->getConfigForm($form, $form_state);
      break;

  }

  $form['#apple_news_destination'] = $destination;

  $form['actions'] = [
    '#type' => 'actions',
    'submit' => [
      '#type' => 'submit',
      '#value' => t('Save changes'),
    ],
    'cancel' => [
      '#markup' => l(t('Cancel'), 'admin/config/content/apple-news/exports/' . apple_news_export_id($export->getModule(), $export->getMachineName())),
    ],
  ];

  return $form;
}

/**
 * Form validatation handler.
 */
function apple_news_export_destination_edit_form_validate($form, &$form_state) {
  /** @var ApplenewsDestination $destination */
  $destination = $form['#apple_news_destination'];
  $destination->validateConfigForm($form, $form_state);
}

/**
 * Form submit handler.
 */
function apple_news_export_destination_edit_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  /** @var ApplenewsDestination $destination */
  $destination = $form['#apple_news_destination'];
  $destination->submitConfigForm($form, $form_state);
  apple_news_export_save($destination->export);
  drupal_set_message(t('Changes have been successfully saved.'));
}

/**
 * Menu callback to delete an export destination.
 *
 * @param ApplenewsExportConfigurable $export
 *   Export object.
 */
function apple_news_export_destination_delete(ApplenewsExportConfigurable $export, $type, $id) {

  switch ($type) {

    case 'components':
      /** @var ApplenewsExport $export */
      $export->deleteComponent($id);
      apple_news_export_save($export);
      drupal_set_message(t('Destination deleted.'));
      break;

  }

  drupal_goto('admin/config/content/apple-news/exports/' . apple_news_export_id($export->getModule(), $export->getMachineName()));
}


/**
 * Delete Apple News article w/ confirmation.
 */
function apple_news_settings_delete_article_form($form, &$form_state, $entity_type, $entity_id, $article_id, $channel_id) {
  $channels = variable_get('apple_news_variable_api_channels', []);

  // Load entity
  $entity = entity_load_single($entity_type, $entity_id);
  $wrapper = entity_metadata_wrapper($entity_type, $entity);
  $entity_uri = entity_uri($entity_type, $entity);
  $path = $entity_uri['path'] . '/edit';
  $title = $wrapper->label();

  $form_state['storage'] = [
    'entity_type' => $entity_type,
    'entity_id' => $entity_id,
    'article_id' => $article_id,
    'channel_id' => $channel_id,
    'channel_name' => $channels[$channel_id]['name'],
    'return_path' => $path,
    'title' => $title
  ];

  return confirm_form(
    $form,
    t("Do you really want to delete <em>@title</em> article from <em>@channel</em>?",
      [
        '@title' => $title,
        '@channel' => $channels[$channel_id]['name']
      ]
    ),
    $path,
    t("This action cannot be undone."),
    t("Yes, delete")
  );
}

/**
 * Form submit handler.
 */
function apple_news_settings_delete_article_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news', 'push_api');

  // Delete an Apple News article.
  $entity = entity_load_single($form_state['storage']['entity_type'], $form_state['storage']['entity_id']);
  $success = apple_news_op('delete', $form_state['storage']['channel_id'], $entity, $form_state['storage']['entity_type']);
  // If successful request.
  if ($success) {
    drupal_set_message(
      t('<em>@title</em> successfully removed from <em>@channel</em>',
        [
          '@title' => $form_state['storage']['title'],
          '@channel' => $form_state['storage']['channel_name'],
        ]
      )
    );
  }
  else {
    drupal_set_message(
      t('Unable to delete <em>@title</em> from <em>@channel</em>',
        [
          '@title' => $form_state['storage']['title'],
          '@channel' => $form_state['storage']['channel_name'],
        ]
      ),
      'warning'
    );
  }
  drupal_goto($form_state['storage']['return_path']);
}
