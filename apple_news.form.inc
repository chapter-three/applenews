<?php
/**
 * @file
 * Form hook implementations.
 */

/**
 * Implements hook_form_alter().
 */
function apple_news_form_alter(&$form, &$form_state, $form_id) {

  // Check if all required libraries installed, otherwise
  // display message about missing PHP libraries.
  module_load_include('inc', 'apple_news');
  if ($info = apple_news_libraries_installed()) {
    $form['apple_news'] += $info;
    return;
  }

  $menu_item = menu_get_item();
  switch ($menu_item['page_callback']) {

    case 'node_page_edit':
    case 'node_add':
      _apple_news_form_alter_node_form($form, $form_state);
      break;

  }
}

/**
 * Entity edit form alter callback.
 */
function _apple_news_form_alter_node_form(&$form, &$form_state) {
  $entity_type = 'node';

  // Entity object.
  if ($entity = menu_get_object()) {
    $entity = menu_get_object();
    $entity_id = $entity->nid;
    // Use entity ID value as revision ID.
    $revision_id = $entity_id;
  };

  // Entity form JS helper (hide/show Apple News entity form options).
  $form['#attached']['js'][] = drupal_get_path('module', 'apple_news') . '/js/form.js';

  module_load_include('inc', 'apple_news');
  $exports = apple_news_exports_enabled();
  $default_export = empty($entity_id) ? NULL
    : apple_news_entity_get_export($entity_type, $entity_id);
  $channels_all = apple_news_channels();

  // Load existing Apple News article.
  $api_channels = [];
  if (!empty($entity_id)) {
    $post = new ApplenewsDatabase();
    if (!empty($channels_all)) {
      $channel_ids = array_keys($channels_all);
      $post->setChannelId($channel_ids);
    }
    $post->setPostId($entity_type, $entity_id, $revision_id);
    $article = $post->getArticle();
    $article_id = !empty($article['article_id']) ? $article['article_id'] : NULL;
    $api_channels = apple_news_channels(TRUE);
  }

  // Add Apple News vertical tab.
  $form['apple_news'] = [
    '#type' => 'fieldset',
    '#title' => t('Apple News'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#weight' => -10,
    '#tree' => TRUE,
    '#attributes' => [
      'class' => ['apple-news-options'],
    ]
  ];

  // Apple News status/publish flag.
  if (!empty($entity->changed) && !empty($article_id) && !empty($article['postdate'])) {
    $label = t('Publish updates to Apple News');
    if ($entity->created != $entity->changed && $entity->changed != $article['postdate']) {
      $description = t('The article has local changes that are not published to Apple News.');
    }
    else {
      $description = t('Apple News published article is in sync with the current content.');
    }
  }
  else {
    $label = t('Publish new content to Apple News');
    $description = '';
  }
  $form['apple_news']['apple_news_publish_flag'] = [
    '#type' => 'checkbox',
    '#title' => $label,
    '#prefix' => '<div><strong>' . $description . '</strong></div>',
    '#description' => t('Check this option if you would like to post this content to Apple News.'),
  ];
  if (@$default_export->enabled && count($api_channels)) {
    $form['apple_news']['apple_news_publish_flag']['#default_value']
      = !empty($article_id) ? TRUE : FALSE;
  }
  elseif (!empty($entity_id)) {
    $form['apple_news']['apple_news_publish_flag']['#disabled'] = TRUE;
  }

  // Override revision.
  if (!empty($article_id)) {
    $form['apple_news']['override_revision'] = [
      '#type' => 'checkbox',
      '#title' => t('Override Apple News article revision'),
      '#description' => t('Check this option if you would like to override current Apple News article revision.'),
      '#states' => [
        'visible' => [
          ':input[name="apple_news_publish_flag"]' => ['checked' => TRUE],
        ],
      ],
    ];
  }

  if (!empty($article_id) && count($api_channels)) {

    $form['apple_news']['post_date'] = [
      '#markup' => '<div class="apple-news-post-date">' . format_date($article['postdate'], 'short') . '</div><br/>',
      '#prefix' => '<div><strong>' . t('Apple News post date') . '</strong></div>',
    ];

    $form['apple_news']['share-url'] = [
      '#markup' => '<div class="apple-news-share-url">' . l($article['share_url'], $article['share_url'],
          [
            'attributes' => [
              'target' => '_blank'
            ]
          ]
        )
        . '</div><br/>',
      '#prefix' => '<div><strong>' . t('Share URL') . '</strong></div>',
    ];

    $form['apple_news']['delete'] = [
      '#markup' => '<div class="apple-news-delete-link">' . t('<a href="!link">Delete</a> this article from Apple News.',
          [
            '!link' => url('admin/config/content/apple-news/'
              . $entity_type . '/'
              . $entity_id
              . '/' . $article['article_id'] . '/'
              . reset($channel_ids)
              . '/delete'
            ),
          ]
        )
        . '</div><br/>',
      '#prefix' => '<div><strong>' . t('Delete') . '</strong></div>',
    ];

  }

  // Exports.
  if (!empty($default_export)) {
    $default_value = apple_news_export_id($default_export->getModule(),
      $default_export->getMachineName());
  }
  else {
    $default_value = NULL;
  }
  $options = [];
  foreach ($exports as $key => $info) {
    if ($key != $default_value) {
      $export = apple_news_get_export($info['module'], $info['machine_name']);
      if (isset($entity_id) &&
          !apple_news_entity_export_valid($export, $entity_type, $entity_id)
      ) {
        continue;
      }
    }
    $options[$key] = $info['name'] . ' (' . $info['module'] . ')';
  }
  switch (count($options)) {

    case 0:
      if (count($exports)) {
        $msg = t('No exports are !link for this node.',
          ['!link' => l(t('configured'), 'admin/config/content/apple-news')]);
      }
      else {
        $msg = t('No exports are !link.',
          ['!link' => l(t('enabled'), 'admin/config/content/apple-news')]);
      }
      $form['apple_news']['exports'] = [
        '#markup' => '<div class="apple-news-preview">' . $msg . '</div><br/>',
      ];
      break;

    case 1:
    default:
      $form['apple_news']['exports'] = [
        '#type' => 'select',
        '#title' => t('Export'),
        '#options' => $options,
        '#default_value' => $default_value,
      ];
      break;

  }

  if (@$default_export->enabled) {
    // Get configuration form for the AppleNews exportable entity.
    $export_form = $default_export->getConfigForm([], $form_state);

    if (isset($export_form['channels'])) {
      foreach (element_children($export_form['channels']) as $key) {
        unset($export_form['channels'][$key]['#default_value']);
      }
      // Set Apple News default values for current entity.
      if (!empty($article_id) && count($api_channels)) {
        foreach ($article['sections'] as $section_id => $channel_id) {
          $key = 'channel-' . $channel_id;
          if (!empty($export_form['channels'][$key])) {
            $export_form['channels'][$key]['#default_value'] = TRUE;
          }
          $key = 'section-' . $channel_id . '-' . $section_id;
          if (!empty($export_form['channels'][$key])) {
            $export_form['channels'][$key]['#default_value'] = TRUE;
          }
        }
      }
      $form['apple_news']['channels'] = $export_form['channels'];

    }

    // Preview-able download.
    if (@$default_export->enabled) {
      $msg = t('<a href="!link">Download</a> the Apple News generated document (use the News Preview app to preview the article).',
        [
          '!link' => url('admin/config/content/apple-news/' . $entity_type . '/' . $entity_id . '/' . $revision_id . '/download'),
        ]
      );
      $form['apple_news']['preview'] = [
        '#markup' => '<div class="apple-news-preview">' . $msg . '</div><br/>',
        '#prefix' => '<div><strong>' . t('Preview') . '</strong></div>',
      ];
    }

  }

  $form['#submit'][] = '_apple_news_form_alter_node_form_submit';

}
