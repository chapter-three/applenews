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

  // Check if all required libraries installed, otherwise
  // display message about missing PHP libraries.
  if ($info = apple_news_libraries_installed()) {
    return $info;
  }

  $form['credentials'] = [
    '#type' => 'fieldset',
    '#title' => t('Apple News Credentials'),
    '#description' => t('You can find your connection information in News Publisher. Go to Channel Info tab to view your API Key and Channel ID.'),
  ];

  $endpoint = variable_get('apple_news_variable_api_endpoint', '');
  $api_key = variable_get('apple_news_variable_api_key', '');
  $api_secret = variable_get('apple_news_variable_api_secret', '');
  $curl_opts = variable_get('apple_news_variable_api_curlopt',
    [
      'ssl'        => TRUE,
      'proxy'      => '',
      'proxy_port' => '',
    ]
  );

  $form['credentials']['apple_news_variable_api_endpoint'] = [
    '#type' => 'textfield',
    '#title' => t('API Endpoint URL'),
    '#default_value' => variable_get('apple_news_variable_api_endpoint', ''),
    '#description' => t('Publisher API endpoint URL'),
  ];

  $form['credentials']['apple_news_variable_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('API Key ID'),
    '#default_value' => $api_key,
    '#description' => t('Publisher API Key ID'),
  ];

  $form['credentials']['apple_news_variable_api_secret'] = [
    '#type' => 'password',
    '#title' => t('API Secret Key'),
    '#default_value' => $api_secret,
    '#description' => t('Publisher API Secret Key'),
  ];

  $form['advanced'] = [
    '#type' => 'fieldset',
    '#title' => t('Advanced'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];

  $form['advanced']['ssl_verification'] = [
    '#type' => 'checkbox',
    '#title' => t('SSL Verification'),
    '#default_value' => $curl_opts['ssl'],
    '#description' => t('Disabling verification makes the communication insecure.'),
  ];

  $form['advanced']['proxy'] = [
    '#type' => 'textfield',
    '#title' => t('Proxy address'),
    '#default_value' => $curl_opts['proxy'],
    '#description' => t('Proxy server address.'),
  ];

  $form['advanced']['proxy_port'] = [
    '#type' => 'textfield',
    '#title' => t('Proxy port'),
    '#default_value' => $curl_opts['proxy_port'],
    '#size' => 10,
    '#description' => t('Proxy server port number.'),
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
            ));
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
  // SSL Verification.
  variable_set('apple_news_variable_api_curlopt',
    [
      'ssl'        => !empty($form_state['values']['ssl_verification']) ? TRUE : FALSE,
      'proxy'      => !empty($form_state['values']['proxy']) ? filter_xss($form_state['values']['proxy']) : '',
      'proxy_port' => !empty($form_state['values']['proxy_port']) ? filter_xss($form_state['values']['proxy_port']) : '',
    ]
  );
  $api_channels = variable_get('apple_news_variable_api_channels', []);
  $channel_msg = '';
  if (empty($api_channels)) {
    $link = l(t('add Channel ID'),
      'admin/config/content/apple-news/settings/channels');
    $channel_msg = t('Please !link to verify connection and start posting content to Apple News.', ['!link' => $link]);
  }
  drupal_set_message(t('The configuration options have been saved. !msg',
    ['!msg' => $channel_msg]));
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
    variable_del($key);
  }
  // Remove data from custom cache table.
  db_truncate('cache_apple_news')->execute();
  // Remove associated data.
  $chnnels_all = apple_news_channels();
  foreach ($chnnels_all as $channel_id => $name) {
    $channel = apple_news_channel($channel_id);
    apple_news_cleanup_data($channel);
  }
  // Delete settings.
  variable_del('apple_news_variable_api_channels');
  variable_del('apple_news_variable_api_curlopt');
  drupal_set_message(t('The configuration options have been deleted.'));
  drupal_goto('admin/config/content/apple-news/settings');
}

/**
 * Form builder for API config.
 */
function apple_news_settings_channels_form() {
  module_load_include('inc', 'apple_news');

  // Check if all required libraries installed, otherwise
  // display message about missing PHP libraries.
  if ($info = apple_news_libraries_installed()) {
    return $info;
  }

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
      'operations' => l(t('Delete'), 'admin/config/content/apple-news/settings/channels/' . $channel->id . '/delete')
    ];
  }

  // Show delete button only when all the fields are prepopulated.
  if (!empty($endpoint) && !empty($api_key) && !empty($api_secret)) {

    $form['channels'] = [
      '#markup' => '<br/>' . theme('table',
        [
          'header' => [
            t('Channel Name'),
            t('Channel ID'),
            t('Operations'),
          ],
          'rows' => $channels,
          'empty' => t('Please add at least one Channel ID in order to be able to publish content to Apple News.'),
        ]
      )
    ];

    $form['apple_news_channel_id'] = [
      '#type' => 'textfield',
      '#title' => t('Channel ID'),
      '#description' => t("You can find your Channel ID in News Publisher. Go to Channel Info tab to view your API Key and Channel ID."),
      '#required' => TRUE,
    ];

    $form['submit'] = [
      '#type' => 'submit',
      '#value' => t('Add new channel'),
    ];

  }
  else {

    $link = l(t('credentials'), 'admin/config/content/apple-news/settings');
    $form['requirements'] = [
      '#markup' => t('Please provide Apple News !link in order to be able to see and add channels.', ['!link' => $link]),
    ];

  }

  return $form;
}

/**
 * Form submit handler.
 */
function apple_news_settings_channels_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  $channel_id = trim($form_state['values']['apple_news_channel_id']);
  $channels = variable_get('apple_news_variable_api_channels', []);

  if ($channel = apple_news_channel($channel_id)) {
    // Save settings.
    if (!isset($channel->errors)) {
      // Get sections.
      apple_news_sections($channel->id);
      // Save new channel ID.
      $channels[] = $channel->id;
      variable_set('apple_news_variable_api_channels', $channels);
      $msg = t('Successfully connected to the "@channel" channel.', [
        '@channel' => filter_xss($channel->name),
      ]);
      drupal_set_message($msg, 'status');
    }
    else {
      $msg = t('There was an error communicating with the server. Please make sure you have correct channel ID and Apple News credentials.');
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
      $default = ($section->isDefault) ? ' ' . t('<em>(default)</em>') : '';
      $sections[] = [
        'name' => filter_xss($section->name) . $default,
        'id'   => $section->id,
      ];
    }
  }

  $form['sections'] = [
    '#markup' => theme('table',
      [
        'header' => [
          t('Section Name'),
          t('Section ID')
        ],
        'rows' => $sections
      ]
    ),
  ];

  $form['back'] = [
    '#markup' => l(t('Back to channels'), 'admin/config/content/apple-news/settings/channels'),
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
    t("Do you really want to delete this channel?"),
    'admin/config/content/apple-news/settings/channels',
    t("You will no long be able to publish content to <em>@name</em> channel. This action cannot be undone.",
      [
        '@name' => $channel->name
      ]),
      t('Yes, delete')
  );
}

/**
 * Form submit handler.
 */
function apple_news_settings_delete_channel_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  $channel = $form_state['storage']['channel'];
  if (empty($channel->id)) {
    drupal_set_message(t('You are trying to remove non-existing channel.'), 'warning');
  }
  else {

    // Remove all channel ID related data.
    apple_news_cleanup_data($channel);
    drupal_set_message(t('<em>@name</em> channel successfully removed.', ['@name' => filter_xss($channel->name)]));

    drupal_goto('admin/config/content/apple-news/settings/channels');
  }
}

/**
 * Form builder for export config.
 */
function apple_news_settings_exports_form($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  // Check if all required libraries installed, otherwise
  // display message about missing PHP libraries.
  if ($info = apple_news_libraries_installed()) {
    return $info;
  }

  $header = [
    'title'       => ['data' => t('Title')],
    'module'      => ['data' => t('Module')],
    'enabled'     => ['data' => t('Enabled')],
    'status'      => ['data' => t('Status')],
    'lastexport'  => ['data' => t('Last Bulk Export')],
    'edit'        => ['data' => ''],
  ];

  $rows = [];
  foreach (apple_news_exports() as $key => $info) {
    $export = apple_news_export_load($key);

    if ($export instanceof ApplenewsExportConfigurable) {
      $export->validateComponents();
    }

    // Get current export status.
    $status = apple_news_batch_export_get_status($export->id);

    /** @var SelectQuery $query */
    $query = db_select('apple_news_log', 'l')
      ->fields('l', ['starttime', 'endtime'])
      ->condition('l.exid', $export->id)
      ->orderBy('l.starttime', 'DESC')
      ->range(0, 1);
    $log = $query->execute()->fetchAssoc();

    $title = $export->title;
    $module = system_get_info('module', $export->getModule());
    $module = $module['name'] . ' (' . $export->getMachineName() . ')';
    $enabled = empty($export->enabled) ? t('no') : t('yes');
    $lastexport = empty($log['starttime']) ? t('never') : format_date($log['starttime'], 'short');
    $edit = l(t('edit'), 'admin/config/content/apple-news/export/' . $key . '/edit');
    $delete = l(t('delete'), 'admin/config/content/apple-news/export/' . $key . '/delete');
    $rows[$key] = [
      'title'       => '<div class="title">' . $title . '</div>' 
                          . (!empty($export->description) ? '<div class="description">'
                          . $export->description . '</div>' : ''),
      'module'      => $module,
      'enabled'     => $enabled,
      'status'      => !empty($status['status']) ? $status['status'] : '',
      'lastexport'  => $lastexport,
      'edit'        => $edit . ' ' . $delete,
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
        'enable'  => t('Enable'),
        'disable' => t('Disable'),
        'export'  => t('Export'),
        'export-to-file' => t('Export to File'),
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
    form_set_error('exports', 'Select at least one export.');
  }

}

/**
 * Form submit handler.
 */
function apple_news_settings_exports_form_submit($form, &$form_state) {

  module_load_include('inc', 'apple_news', 'apple_news.publisher_api');
  module_load_include('inc', 'apple_news');

  $exports = apple_news_exports();
  $selected = array_map(function($key) use(&$exports) {
    return $exports[$key];
  }, array_filter($form_state['values']['exports']));

  foreach (array_keys($selected) as $id) {
    $export = apple_news_export_load($id);

    switch ($form_state['values']['action']) {

      case 'enable':
        $export->enabled = TRUE;
        apple_news_export_save($export);
        drupal_set_message(
          t('Export %title sucessfully <strong>enabled</strong>.',
            ['%title' => $export->title]));
        break;

      case 'disable':
        $export->enabled = FALSE;
        apple_news_export_save($export);
        drupal_set_message(
          t('%title export sucessfully <strong>disabled</strong>.',
            ['%title' => $export->title]));
        break;

      case 'export':
        // Batch entity export to Apple News.
        if ($export->enabled) {
          $batch = apple_news_batch_run_export($export, 'export');
          batch_set($batch);
        }
        else {
          drupal_set_message(
            t('%title export is disabled.',
              ['%title' => $export->title]),
            'warning'
          );
        }
        break;

      case 'export-to-file':
        // Export entities as Apple News previewable files.
        if ($export->enabled) {
          $batch = apple_news_batch_run_export($export, 'export-to-file');
          batch_set($batch);
        }
        else {
          drupal_set_message(
            t('%title export is disabled.',
              ['%title' => $export->title]),
            'warning'
          );
        }
        break;

    }
  }
}

/**
 * Form builder for a new export.
 */
function apple_news_export_add_form($form) {
  module_load_include('inc', 'apple_news');

  $form['type'] = [
    '#type' => 'radios',
    '#title' => t('Type'),
  ];
  $implementations = apple_news_get_module_apis('export');
  array_walk($implementations, function($info, $key) use(&$form) {
    $form['type']['#options'][$key] = $info['name'] . ' - ' .
      $info['description'];
  });
  $options = array_keys($form['type']['#options']);
  $form['type']['#default_value'] = reset($options);

  $form['title'] = [
    '#type' => 'textfield',
    '#title' => t('Administrative Title'),
    '#required' => TRUE,
  ];

  $form['actions'] = [
    '#type' => 'actions',
    'submit' => [
      '#type' => 'submit',
      '#value' => t('Submit'),
    ],
    'cancel' => [
      '#markup' => l(t('Back'), 'admin/config/content/apple-news'),
    ],
  ];

  return $form;
}

/**
 * Form validatation handler.
 */
function apple_news_export_add_form_validate($form, &$form_state) {
  if (empty($form_state['values']['type'])) {
    form_set_error('exports', 'Select a type.');
  }
}

/**
 * Form submit handler.
 */
function apple_news_export_add_form_submit($form, &$form_state) {
  list($module, $machine_name) = _apple_news_export_id_extract(
    $form_state['values']['type']);
  $export = apple_news_new_export($module, $machine_name);
  $export->title = $form_state['values']['title'];
  apple_news_export_save($export);
  drupal_goto('admin/config/content/apple-news/export/' . $export->id);
}

/**
 * Form builder for export delete.
 *
 * @param ApplenewsExportConfigurable $export
 *   Export object.
 */
function apple_news_export_delete_form($form, &$form_state, ApplenewsExportConfigurable $export) {

  $form_state['storage'] = [
    'export'      => $export,
  ];

  return confirm_form(
    $form,
    t("Do you really want delete the export !title?",
      ['!title' => $export->title]),
    'admin/config/content/apple-news/export/' . $export->id . '/edit',
    t("This action cannot be undone."),
    t("Yes, delete")
  );
}

/**
 * Form submit handler.
 */
function apple_news_export_delete_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  /** @var ApplenewsExportConfigurable $export */
  $export = $form_state['storage']['export'];
  $path = 'admin/config/content/apple-news/exports';

  if (apple_news_export_delete($export)) {
    drupal_set_message(t('%name successfully deleted.',
      ['%name' => $export->title]));
  }
  drupal_goto($path);

}

/**
 * Form builder for an export config.
 *
 * @param ApplenewsExport $export
 *   Export object.
 */
function apple_news_export_edit_form($form, &$form_state, ApplenewsExport $export) {
  module_load_include('inc', 'apple_news');

  if ($export instanceof ApplenewsExportConfigurable) {
    $export->validateComponents();
  }

  $form = $export->getConfigForm($form, $form_state);

  $form['#apple_news_export'] = $export;

  $form['actions'] = [
    '#type' => 'actions',
    'submit' => [
      '#type' => 'submit',
      '#value' => t('Save changes'),
    ],
    'cancel' => [
      '#markup' => l(t('Back'), 'admin/config/content/apple-news'),
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

  // Non-order-able table of destinations.
  $elements = [
    [
      'item_label' => t('Component Layout'),
      'element' => &$form['additional_settings']['layouts_tab']['component_layouts'],
    ],
  ];
  foreach ($elements as $info) {
    $rows = [];
    foreach (element_children($info['element']) as $id) {
      $rows[] = [
        'data' => [
          drupal_render($info['element'][$id]['destination']),
          drupal_render($info['element'][$id]['edit']),
          drupal_render($info['element'][$id]['delete']),
          drupal_render($info['element'][$id]['id']),
        ],
      ];
    }

    $header = [
      $info['item_label'],
      ['data' => t('Operations'), 'colspan' => 2],
    ];
    $info['element'] = [
      '#markup' => theme('table', ['header' => $header, 'rows' => $rows]),
      '#weight' => @$info['element']['#weight'] ? $info['element']['#weight'] : 0,
      '#access' => isset($info['element']['#access']) ? $info['element']['#access'] : TRUE,
    ];
  }

  // Order-able sets of destinations.
  $elements = [
    'components' => [
      'item_label' => t('Component'),
      'element' => &$form['additional_settings']['components_tab']['components'],
    ],
  ];
  foreach ($elements as $key => $info) {
    $css_prefix = "apple-news-${key}-";

    $rows = [];
    foreach (element_children($info['element']) as $id) {
      $info['element'][$id]['weight']['#attributes']['class'] = [$css_prefix . 'weight'];
      $info['element'][$id]['id']['#attributes']['class'] = [$css_prefix . 'id'];
      $info['element'][$id]['pid']['#attributes']['class'] = [$css_prefix . 'pid'];
      $class = ['draggable'];
      if (@$info['element'][$id]['#apple_news_destination']) {
        /** @var ApplenewsDestination $destination */
        $destination = $info['element'][$id]['#apple_news_destination'];
        if ($destination->isRoot()) {
          $class[] = 'tabledrag-root';
        }
        if ($destination->isLeaf()) {
          $class[] = 'tabledrag-leaf';
        }
      }
      $indent = theme('indentation', ['size' => $info['element'][$id]['depth']['#value']]);
      unset($info['element'][$id]['depth']);
      $rows[] = [
        'data' => [
          $indent . drupal_render($info['element'][$id]['destination']),
          drupal_render($info['element'][$id]['edit']),
          drupal_render($info['element'][$id]['delete']),
          drupal_render($info['element'][$id]['weight']) .
          drupal_render($info['element'][$id]['id']) .
          drupal_render($info['element'][$id]['pid']),
        ],
        'class' => $class,
      ];
    }

    $header = [
      $info['item_label'],
      ['data' => t('Operations'), 'colspan' => 2],
      t('Weight'),
    ];
    $table_id = $css_prefix . 'table';
    $table = [
      'header' => $header,
      'rows' => $rows,
      'attributes' => ['id' => $table_id],
    ];
    $info['element'] = [
      '#markup' => theme('table', $table),
      '#weight' => @$info['element']['#weight'] ? $info['element']['#weight'] : 0,
      '#access' => isset($info['element']['#access']) ? $info['element']['#access'] : TRUE,
    ];
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

  return drupal_render_children($form);
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
  if (empty($form_state['redirect'])) {
    drupal_set_message(t('Changes have been successfully saved.'));
  }
}

/**
 * Form builder for an export config.
 *
 * @param ApplenewsExportConfigurable $export
 *   Export object.
 */
function apple_news_export_destination_edit_form($form, &$form_state, ApplenewsExportConfigurable $export, $type, $id) {

  switch ($type) {

    case 'component':
      /** @var ApplenewsDestinationComponent $destination */
      $destination = $export->getComponent($id);
      drupal_set_title(t('Edit Component: !label', [
        '!label' => check_plain($destination->label()),
      ]), PASS_THROUGH);
      break;

    case 'layout':
      $destination = $export->getLayout();
      $form['destination_label'] = [
        '#markup' => '<div class="form-item"><label for="edit-weight">' . t('Layout') . ' </label>
        ' . $destination->label() . '
        </div>',
      ];
      break;

    case 'componentLayout':
      /** @var ApplenewsDestinationComponentLayout $destination */
      $destination = $export->getComponentLayout($id);
      $form['destination_label'] = [
        '#markup' => '<div class="form-item"><label for="edit-weight">' . t('Component Layout') . ' </label>
        ' . $destination->label() . '
        </div>',
      ];
      break;

    default:
      return $form;

  }

  $form = $destination->getConfigForm($form, $form_state);

  $form['#apple_news_destination'] = $destination;

  $form['actions'] = [
    '#type' => 'actions',
    'submit' => [
      '#type' => 'submit',
      '#value' => t('Save changes'),
    ],
    'cancel' => [
      '#markup' => l(t('Back'), 'admin/config/content/apple-news/export/' . $export->id, [
          // Set components tab as Active.
          'fragment' => 'edit-components-tab',
        ]
      ),
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
  $form_state['redirect'] = [
    'admin/config/content/apple-news/export/' . $destination->export->id, [
      // Set components tab as Active.
      'fragment' => 'edit-components-tab',
    ]
  ];
}

/**
 * Forum builder for delete an export destination.
 *
 * @param ApplenewsExportConfigurable $export
 *   Export object.
 */
function apple_news_export_destination_delete_form($form, &$form_state, ApplenewsExportConfigurable $export, $type, $id) {
  module_load_include('inc', 'apple_news');

  $path = 'admin/config/content/apple-news/export/' . $export->id . '/edit';

  switch ($type) {

    case 'component':
      $destination = $export->getComponent($id);
      break;

    case 'layout':
      $destination = $export->getLayout();
      break;

    case 'componentLayout':
      $destination = $export->getComponentLayout($id);
      break;

    default:
      drupal_goto($path, [
          // Set components tab as Active.
          'attributes' => [
            'fragment' => 'edit-components-tab',
          ]
        ]
      );
      return NULL;

  }

  $form_state['storage'] = [
    'export'      => $export,
    'type'        => $type,
    'id'          => $id,
    'destination' => $destination,
  ];

  return confirm_form(
    $form,
    t("Do you really want delete <em>!name</em>?",
      ['!name' => $destination->label()]),
    [
      'path' => $path,
      // Set components tab as Active.
      'fragment' => 'edit-components-tab',
    ],
    t("This action cannot be undone."),
    t("Yes, delete")
  );
}

/**
 * Form submit handler.
 */
function apple_news_export_destination_delete_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');

  /** @var ApplenewsExportConfigurable $export */
  $export = $form_state['storage']['export'];
  $type = $form_state['storage']['type'];
  $id = $form_state['storage']['id'];
  /** @var ApplenewsDestination $destination */
  $destination = $form_state['storage']['destination'];
  $path = 'admin/config/content/apple-news/export/' . $export->id;

  switch ($type) {

    case 'component':
      $export->deleteComponent($id);
      break;

    case 'layout':
      // No-op.
      drupal_goto($path);
      break;

    case 'componentLayout':
      $export->deleteComponentLayout($id);
      break;

  }

  apple_news_export_save($export);
  drupal_set_message(t('!name successfully deleted.',
    ['!name' => $destination->label()]));
  $form_state['redirect'] = [
    $path,
    [
      // Set components tab as Active.
      'fragment' => 'edit-components-tab',
    ]
  ];

}

/**
 * Delete Apple News article w/ confirmation.
 */
function apple_news_settings_delete_article_form($form, &$form_state, $entity_type, $entity_id, $article_id, $channel_id) {
  module_load_include('inc', 'apple_news');
  // Load channel information.
  $channel = apple_news_channel($channel_id);
  // Load entity.
  $entity = entity_load_single($entity_type, $entity_id);
  $wrapper = entity_metadata_wrapper($entity_type, $entity);
  $entity_uri = entity_uri($entity_type, $entity);
  $path = $entity_uri['path'] . '/edit';
  $title = $wrapper->label();

  $form_state['storage'] = [
    'entity_type'  => $entity_type,
    'entity_id'    => $entity_id,
    'article_id'   => $article_id,
    'channel_id'   => $channel_id,
    'channel_name' => $channel->name,
    'return_path'  => $path,
    'title'        => $title
  ];

  $msg = t("Do you really want to delete <em>@title</em> article from <em>@channel</em>?", [
    '@title'   => $title,
    '@channel' => $channel->name,
  ]);
  return confirm_form(
    $form,
    $msg,
    $path,
    t("This action cannot be undone."),
    t("Yes, delete")
  );
}

/**
 * Form submit handler.
 */
function apple_news_settings_delete_article_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news', 'apple_news.publisher_api');

  // Delete an Apple News article.
  $entity = entity_load_single($form_state['storage']['entity_type'], $form_state['storage']['entity_id']);
  $success = apple_news_op('delete', $form_state['storage']['channel_id'], $entity, $form_state['storage']['entity_type']);
  // If successful request.
  if ($success) {
    drupal_set_message(
      t('<em>@title</em> successfully removed from <em>@channel</em>',
        [
          '@title'   => $form_state['storage']['title'],
          '@channel' => $form_state['storage']['channel_name'],
        ]));
  }
  else {
    drupal_set_message(
      t('Unable to delete <em>@title</em> from <em>@channel</em>',
        [
          '@title'   => $form_state['storage']['title'],
          '@channel' => $form_state['storage']['channel_name']
        ]), 'warning'
    );
  }
  drupal_goto($form_state['storage']['return_path']);
}

/**
 * Download article.json preview file.
 */
function apple_news_download_article_json($entity_type, $entity_id, $revision_id) {
  module_load_include('inc', 'apple_news');
  if (!empty($entity_id)) {

    if ($export = apple_news_entity_get_export($entity_type, $entity_id)) {
      $data = apple_news_entity_export($export, $entity_type, $entity_id);
      apple_news_export_to_file($entity_id, [$entity_id], NULL, TRUE, $data);
      $archive_path = apple_news_export_file_path($entity_id);

      $archive = $archive_path . '.zip';
      $output = file_get_contents($archive, FILE_USE_INCLUDE_PATH);

      // Remove individual entity archive.
      apple_news_export_file_delete($entity_id);
      // Remove individual entity directory.
      apple_news_export_dir_cleanup([$entity_id]);

      drupal_add_http_header('Content-Type', 'application/zip');
      drupal_add_http_header('Content-Disposition', 'attachment; filename=' . $entity_id . '.zip');

      echo $output;

    }
    else {
      return t('Export not defined for this entity.');
    }
  }
  else {
    return t('Entity not found.');
  }

  drupal_exit();
  return NULL;
}

/**
 * Autoinstall form.
 */
function apple_news_autoinstall_form($form, &$form_state) {
  $libraries = [
    'php-curl-class' => CURL_SUPPORTED_VERSION,
    'AppleNews'      => APPLE_NEWS_SUPPORTED_VERSION,
  ];
  $count = 0;
  foreach ($libraries as $name => $version) {
    if ($library = libraries_detect($name)) {
      $lib_version = !empty($library['version']) ? $library['version'] : '';
      if ($lib_version == $version) {
        $count++;
      }
    }
  }
  // All libraries successfully installed.
  if ($count == 2) {
    $link = l(t('configure the module'), 'admin/config/content/apple-news/settings');
    $form['info'] = [
      '#markup' => t('Apple News dependant PHP libraries were successfully installed. Please !link and start posting content to Apple News.', ['!link' => $link]),
    ];
  }
  // Show download button.
  else {
    $form['submit'] = [
      '#type' => 'submit',
      '#value' => t('Download and Unzip'),
      '#prefix' => '<div>' . t("This is experimental feature. The module will download and unzip Apple News dependant PHP libraries. Please note this module won't download libraries from private repositories.") . '</div><br/>',
    ];
  }
  return $form;
}

/**
 * Form submit handler.
 */
function apple_news_autoinstall_form_submit($form, &$form_state) {
  module_load_include('inc', 'apple_news');
  // Download and unzip PHP libraries.
  // @see apple_news.inc file.
  apple_news_autoinstall_libraries();
}
