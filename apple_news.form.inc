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

          $published_channels = [];

          // Entity object.
          $entity_id = isset($form['nid']['#value']) ? (int) $form['nid']['#value'] : NULL;
          $article_id = NULL;

          if (!empty($entity_id)) {

            $entity = entity_load_single($type, $entity_id);

            module_load_include('inc', 'apple_news', 'include/db');
            $post = new ChapterThree\AppleNews\Database\ApplenewsDatabase();
            $post->setPostId($type, $entity_id, $entity_id);
            $article = $post->getArticle();

            $article_id = !empty($article['article_id']) ? $article['article_id'] : NULL;

          }

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

          if (empty($entity_id)) {
            $preview_message = t('You have to save this content in order to be able to preview the Apple News generated document.');
          }
          else {
            $preview_message = t('<a href="!link">Download</a> the Apple News generated document.',
              [
                '!link' => url('admin/config/content/apple-news/' . $article_id . '/download'),
              ]
            );
          }

          $form['apple_news']['preview'] = [
            '#markup' => '<div>' . $preview_message . '</div><br/>',
            '#prefix' => '<div><strong>' . t('Preview') . '</strong></div>',
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
            $form['apple_news']['container'] = $export_form['channels'];
          }

          if (!empty($article_id) && count($api_channels)) {

            $rows = [];

            foreach ($article['sections'] as $section_id => $channel_id) {
              $channel = apple_news_channel($channel_id);
              $section = apple_news_section($section_id);

              if (!empty($channel->name)) {
                $rows[] = [
                  ['data' => $channel->name],
                  ['data' => $section['name']],
                  ['data' => 
                    l($article['share_url'], $article['share_url'],
                      [
                        'attributes' => [
                          'target' => '_blank'
                        ]
                      ]
                    )
                  ],
                  ['data' => format_date($article['postdate'], 'sort')],
                ];
              }
            }

            $form['apple_news']['info'] = [
              '#markup' => theme('table',
              [
                'header' => [
                  t('Channel'),
                  t('Section'),
                  t('Share URL'),
                  t('Post date')
                ],
                'rows' => $rows
              ]),
              '#prefix' => '<br/><div><strong>' . t('Apple News feeds') . '</strong></div>',
            ];

            $form['apple_news']['delete'] = [
              '#markup' => '<div>' . t('<a href="!link">Delete</a> this article from Apple News.',
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
            ];

          }

        }
      }
    }

  }
}
