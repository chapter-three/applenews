<?php
/**
 * @file
 * Views hook implementations.
 */

/**
 * Implements hook_views_data().
 */
function apple_news_views_data() {
  module_load_include('inc', 'apple_news');

  // Basic table information.
  $data['apple_news_published_entities']['table']['group'] = t('Apple News Articles');

  // Advertise this table as a possible base table.
  $data['apple_news_published_entities']['table']['base'] = [
    'field' => 'entity_id',
    'title' => t('Apple News Articles'),
    'help' => t('A list of Apple News published articles.'),
    'weight' => 10,
  ];

  $types = apple_news_get_entity_types();
  $entity_types = array_keys($types);
  $fields = ['entity_id', 'revision_id'];

  // Add relationship to entity base via entity_id and revision_id fields.
  $entity_type_exports = apple_news_get_entity_types();
  foreach ($entity_type_exports as $module => $info) {
    foreach ($info['entity_types'] as $entity_type) {
      $entity_info = entity_get_info($entity_type);
      foreach ($fields as $field_name) {

        $data['apple_news_published_entities']['reverse_' . $field_name . '_' . $entity_type]['relationship'] = [
          'handler' => 'views_handler_relationship_entity_reverse',
          'field_name' => $field_name,
          'field table' => 'apple_news_published_entities',
          'field field' => $field_name,
          'base' => $entity_info['base table'],
          'base field' => $entity_info['entity keys']['id'],
          'label' => t('Apple News articles referencing Drupal entities'),
          'group' => t('Apple News Articles'),
          'title' => t('Apple News Referencing !field_name', ['!field_name' => $field_name]),
          'help' => t('A bridge to the @entity_type entity that is referencing Apple News article via !field_name',
            [
              '@entity_type' => $entity_type,
              '!field_name' => $field_name,
            ]
          ),
          'join_extra' => [
            0 => [
              'field' => 'entity_type',
              'value' => $entity_type,
            ],
          ],
        ];

      }
    }
  }

  $data['apple_news_published_entities']['article_id'] = [
    'title' => t('Article ID'),
    'help' => t('Apple News Article ID'),
    'group' => t('Apple News Articles'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  $data['apple_news_published_entities']['article_revision_id'] = [
    'title' => t('Revision ID'),
    'help' => t('Apple News Article Revision ID'),
    'group' => t('Apple News Articles'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  $data['apple_news_published_entities']['channel_id'] = [
    'title' => t('Channel ID'),
    'help' => t('Apple News Channel ID'),
    'group' => t('Apple News Articles'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_string',
    ],
  ];

  $data['apple_news_published_entities']['metadata'] = array(
    'field' => array(
      'title' => t('Apple News article metadata'),
      'group' => t('Apple News Articles'),
      'help' => t('Display Apple News post metadata.'),
      'handler' => 'views_handler_apple_news_metadata',
    ),
  );

  $data['apple_news_published_entities']['postdate'] = [
    'title' => t('Post date'),
    'help' => t('Apple News post timestamp'),
    'group' => t('Apple News Articles'),
    'field' => [
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ],
    'sort' => [
      'handler' => 'views_handler_sort_date',
    ],
    'filter' => [
      'handler' => 'views_handler_filter_date',
    ],
  ];

  return $data;

}
