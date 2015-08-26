<?php
/**
 * @file
 * Form hook implementations.
 */

/**
 * Implements hook_form_alter().
 */
function apple_news_form_alter(&$form, &$form_state, $form_id) {
  module_load_include('inc', 'apple_news');

  // Get all possible entity types and bundles available for export.
  foreach (apple_news_get_entity_types() as $module => $info) {
    if (!isset($info['bundles'])) {
      continue;
    }
    foreach ($info['entity_types'] as $type) {
      foreach ($info['bundles'] as $bundle) {

        if ($form_id == $bundle . '_' . $type . '_form' && $info['enabled']) {

          // Load export object.
          $export = $info['#export'];

          // Entity object.
          $entity_id = isset($form['nid']['#value']) ? (int) $form['nid']['#value'] : NULL;
          $article_id = NULL;

          // Entity form JS helper (hide/show Apple News entity form options).
          $form['#attached']['js'][] = drupal_get_path('module', 'apple_news') . '/js/form.js';

          // Add Apple News vertical tab.
          $form['apple_news'] = [
            '#type' => 'fieldset',
            '#title' => t('Apple News'),
            '#collapsible' => TRUE,
            '#collapsed' => TRUE,
            '#group' => 'additional_settings',
            '#weight' => -10,
            '#attributes' => [
              'class' => ['apple-news-options'],
            ]
          ];

          // Check if all required libraries installed, otherwise
          // display message about missing PHP libraries.
          if ($info = apple_news_libraries_installed()) {
            $form['apple_news'] += $info;
            continue;
          }

          // Previewable download default message.
          $preview_message = t('You have to save this content in order to be able to preview the Apple News generated document.');

          if (!empty($entity_id)) {

            // Load entity object.
            $entity = entity_load_single($type, $entity_id);

            // Use entity ID value as revision ID.
            $revision_id = $entity_id;

            $chnnels_all = apple_news_channels();

            // Load existing Apple News article.
            $post = new ApplenewsDatabase();
            if (!empty($chnnels_all)) {
              $channel_ids = array_keys($chnnels_all);
              $post->setChannelId($channel_ids);
            }
            $post->setPostId($type, $entity_id, $revision_id);
            $article = $post->getArticle();

            $article_id = !empty($article['article_id']) ? $article['article_id'] : NULL;

            // Previewable download.
            $preview_message = t('<a href="!link">Download</a> the Apple News generated document and use the News Preview app to preview the article.',
              [
                '!link' => url('admin/config/content/apple-news/' . $type . '/' . $entity_id . '/' . $revision_id . '/download'),
              ]
            );

          }

          $form['apple_news']['preview'] = [
            '#markup' => '<div class="apple-news-preview">' . $preview_message . '</div><br/>',
            '#prefix' => '<div><strong>' . t('Preview') . '</strong></div>',
            '#weight' => 20,
          ];

          // Get configuration form for the AppleNews exportable entity.
          $export_form = $export->getConfigForm([], $form_state);
          $api_channels = apple_news_channels(TRUE);

          if (isset($export_form['channels'])) {
            // Remove '#required' from configuration form.
            // This should be optional on entity add/edit forms.
            foreach ($api_channels as $channel_id => $channel) {
              $export_form['channels']['channel-' . $channel_id]['#required'] = FALSE;
              $export_form['channels']['channel-' . $channel_id]['#default_value'] = FALSE;
              foreach ($channel['sections'] as $section_id => $section) {
                $export_form['channels']['section-' . $channel_id . '-' . $section_id]['#default_value'] = FALSE;
              }
            }
            // Set Apple News default values for current entity.
            if (!empty($article_id) && count($api_channels)) {
              foreach ($article['sections'] as $section_id => $channel_id) {
                if (!empty($export_form['channels']['channel-' . $channel_id])) {
                  $export_form['channels']['channel-' . $channel_id]['#default_value'] = TRUE;
                }
                if (!empty($export_form['channels']['section-' . $channel_id . '-' . $section_id])) {
                  $export_form['channels']['section-' . $channel_id . '-' . $section_id]['#default_value'] = TRUE;
                }
              }
            }
            $form['apple_news']['container'] = [
              '#type' => 'container',
            ];
            $form['apple_news']['container'] = $export_form['channels'];

            $description = '';
            $label = t('Publish to Apple News');
            if (!empty($entity->changed)) {
              if (!empty($article['postdate'])) {
                if ($entity->created != $entity->changed && $entity->changed != $article['postdate']) {
                  $label = t('Publish updates to Apple News');
                  $description = t('The article has local changes that are not published to Apple News.');
                }
                else {
                  $label = t('Publish updates to Apple News');
                  $description = t('Apple News published article is in sync with the current content.');
                }
              }
              else {
                $label = t('Publish this content to Apple News');
              }
            }
            else {
              $label = t('Publish new content to Apple News');
            }

            // Apple News publish flag.
            if (count($api_channels)) {
              $form['apple_news']['apple_news_publish_flag'] = [
                '#type' => 'checkbox',
                '#title' => $label,
                '#prefix' => '<div><strong>' . $description . '</strong></div>',
                '#default_value' => !empty($article_id) ? TRUE : FALSE,
                '#description' => t('Check this option if you would like to post this content to Apple News.'),
                '#weight' => -14,
              ];
              // Override revision.
              if (!empty($article_id)) {
                $form['apple_news']['apple_news_override_revision'] = [
                  '#type' => 'checkbox',
                  '#title' => t('Override Apple News article revision'),
                  '#description' => t('Check this option if you would like to override current Apple News article revision.'),
                  '#states' => [
                    'visible' => [
                      ':input[name="apple_news_publish_flag"]' => ['checked' => TRUE],
                    ],
                  ],
                  '#weight' => -13,
                ];
              }
            }

          }

          if (!empty($article_id) && count($api_channels)) {

            $form['apple_news']['post_date'] = [
              '#markup' => '<div class="apple-news-post-date">' . format_date($article['postdate'], 'short') . '</div><br/>',
              '#prefix' => '<div><strong>' . t('Apple News post date') . '</strong></div>',
              '#weight' => -12,
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
              '#weight' => 19,
            ];

            $form['apple_news']['delete'] = [
              '#markup' => '<div class="apple-news-delete-link">' . t('<a href="!link">Delete</a> this article from Apple News.',
                [
                  '!link' => url('admin/config/content/apple-news/'
                    . $type . '/'
                    . $entity_id
                    . '/' . $article['article_id'] . '/'
                    . $channel_id
                    . '/delete'
                  ),
                ]
              ) 
              . '</div><br/>',
              '#prefix' => '<div><strong>' . t('Delete') . '</strong></div>',
              '#weight' => 50,
            ];

          }

        }
      }
    }

  }
}
