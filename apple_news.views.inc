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

  // Advertise this table as a possible base table
  $data['apple_news_published_entities']['table']['base'] = array(
    'field' => 'entity_id',
    'title' => t('Apple News Articles'),
    'help' => t('A list of Apple News published articles.'),
    'weight' => 10,
  );

  $types = apple_news_get_entity_types();
  $entity_types = array_keys($types);
  $fields = ['entity_id', 'revision_id'];

  // Add relationship to entity base via entity_id and revision_id fields.
  foreach ($entity_types as $entity_type) {
    $entity_info = entity_get_info('node');
    foreach ($fields as $field_name) {
      $data['apple_news_published_entities']['reverse_entity_id_' . $entity_type]['relationship'] = array(
        'handler' => 'views_handler_relationship_entity_reverse',
        'field_name' => $field_name,
        'field table' => 'apple_news_published_entities',
        'field field' => $field_name,
        'base' => $entity_info['base table'],
        'base field' => $entity_info['entity keys']['id'],
        'label' => t('Apple News articles referencing Drupal entities'),
        'group' => t('Apple News Articles'),
        'title' => t('Apple News Referencing entity'),
        'help' => t('A bridge to the @entity_type entity that is referencing Apple News article via !field_name',
          [
            '@entity_type' => $entity_type,
            '!field_name' => $field_name,
          ]
        ),
        'join_extra' => array(
          0 => array(
            'field' => 'entity_type',
            'value' => $entity_type,
          ),
        ),
      );
    }
  }

  $data['apple_news_published_entities']['article_id'] = array(
    'title' => t('Article ID'),
    'help' => t('Apple News Article ID'),
    'group' => t('Apple News Articles'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['apple_news_published_entities']['channel_id'] = array(
    'title' => t('Channel ID'),
    'help' => t('Apple News Channel ID'),
    'group' => t('Apple News Articles'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['apple_news_published_entities']['postdate'] = array(
    'title' => t('Post date'),
    'help' => t('Apple News post timestamp'),
    'group' => t('Apple News Articles'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  return $data;

}
