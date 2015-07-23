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
  $entity_types = apple_news_get_entity_types();
  foreach ($entity_types as $type => $export_data) {
    foreach ($export_data as $bundle => $info) {
      if ($form_id == $bundle . '_' . $type . '_form') {
        // Connected Apple News channels.
        $channels = apple_news_channels();
        $entity_channels = [];

        // Entity object.
        $entity_id = isset($form['nid']['#value']) ? (int) $form['nid']['#value'] : NULL;

        if (!empty($entity_id)) {
          $entity = entity_load_single($type, $entity_id);
          $results = apple_news_load_articles($entity, $type);
          // Collect article id, channel name and postdate for the table below.
          foreach ($results as $row) {
            $entity_channels[$row->article_id] = [
              'name' => $channels[$row->channel_id],
              'postdate' => format_date($row->postdate, 'short'),
              'article_id' => $row->article_id,
              'channel_id' => $row->channel_id,
            ];
          }
        }

        // Entity form JS helper (hide/show Apple News entity form options).
        $form['#attached']['js'][] = drupal_get_path('module', 'apple_news') . '/js/form.js';

        // Add Apple News vertical tab.
        $form['apple_news'] = array(
          '#type' => 'fieldset',
          '#title' => t('Apple News'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#group' => 'additional_settings',
          '#weight' => -10,
          '#attributes' => array(
            'class' => array('apple-news-options'),
          ),
        );

        $form['apple_news']['post_to_apple_news_channels'] = array(
          '#type' => 'checkboxes',
          '#title' => t('Feeds'),
          '#options' => $channels,
          '#description' => t('Publish this content to selected Apple News Feed.'),
        );

        $export = apple_news_get_export($info['module'], $info['machine_name']);
        $export_config_form = $export->getConfigForm($form, $form_state);

        $form['apple_news']['post_to_apple_news_layout'] = $export_config_form['layout'];
        $form['apple_news']['post_to_apple_news_layout']['#description'] = t('Choose an Apple News layout for this content');

        if (!empty($entity_channels)) {
          // If this article was already published we should give an
          // option to removed it from Apple News.
          $rows = [];
          foreach ($entity_channels as $article_id => $info) {
            $rows[] = [
              ['data' => $info['name']],
              ['data' => $info['postdate']],
              ['data' => l(t('Delete'),
              	  'admin/config/content/apple-news/'
              	    . $type . '/'
              	    . $entity_id . '/'
              	    . $info['article_id'] . '/'
              	    . $info['channel_id'] . '/delete'
              	)
              ]
            ];
          }
          $form['apple_news']['info'] = array(
            '#markup' => theme('table', array(
              'header' => [t('Feed'), t('Post date'), ''],
              'rows' => $rows
            )),
          );
        }
      }
    }
  }
}
