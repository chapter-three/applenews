<?php
/**
 * @file
 * Views hook implementations.
 */

/**
 * Implements hook_views_api().
*/
function apple_news_views_api() {
  return [
    'api' => 3,
  ];
}

/**
 * Implements hook_views_data().
 */
function apple_news_views_data() {
  module_load_include('inc', 'apple_news');

  // Apple News post information.
  $data['apple_news_entities']['table']['group'] = t('Apple News Article');

  $data['apple_news_entities']['table']['base'] = [
    'field' => 'entity_id',
    'title' => t('Apple News Articles'),
    'help' => t('A list of Apple News published articles.'),
    'weight' => 10,
  ];

  $data['apple_news_entities']['post_id'] = [
    'title' => t('Post ID'),
    'help' => t('Apple News post ID'),
    'group' => t('Apple News Article'),
    'field' => [
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
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

        $data['apple_news_entities']['reverse_' . $field_name . '_' . $entity_type]['relationship'] = [
          'handler' => 'views_handler_relationship_entity_reverse',
          'field_name' => $field_name,
          'field table' => 'apple_news_entities',
          'field field' => $field_name,
          'base' => $entity_info['base table'],
          'base field' => $entity_info['entity keys']['id'],
          'label' => t('Apple News articles referencing Drupal entities'),
          'group' => t('Apple News Article'),
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

  $data['apple_news_entities']['article_id'] = [
    'title' => t('Article ID'),
    'help' => t('Apple News Article ID'),
    'group' => t('Apple News Article'),
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

  $data['apple_news_entities']['article_revision_id'] = [
    'title' => t('Revision ID'),
    'help' => t('Apple News Article Revision ID'),
    'group' => t('Apple News Article'),
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

  $data['apple_news_entities']['share_url'] = [
    'title' => t('Share URL'),
    'help' => t('Apple News Article Share URL'),
    'group' => t('Apple News Article'),
    'field' => [
      'handler' => 'views_handler_field_url',
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

  $data['apple_news_entities']['postdate'] = [
    'title' => t('Post date'),
    'help' => t('Apple News post timestamp'),
    'group' => t('Apple News Article'),
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

  // Metadata table.
  $data['apple_news_metadata']['table']['join'] = [
    'apple_news_entities' => [
      'left_field' => 'post_id',
      'field' => 'post_id'
    ]
  ];

  $data['apple_news_metadata']['post_id'] = [
    'title' => t('Post ID'),
    'help' => t('Apple News Post ID  (metadata)'),
    'group' => t('Apple News Article'),
    'field' => [
      'handler' => 'views_handler_field_serialized',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
  ];

  $data['apple_news_metadata']['data'] = [
    'title' => t('Metadata'),
    'help' => t('Apple News metadata'),
    'group' => t('Apple News Article'),
    'field' => [
      'handler' => 'views_handler_field_serialized',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
  ];

  // Channels table.
  $data['apple_news_channels']['table']['join'] = [
    'apple_news_entities' => [
      'left_field' => 'post_id',
      'field' => 'post_id'
    ]
  ];

  $data['apple_news_channels']['post_id'] = [
    'title' => t('Post ID'),
    'help' => t('Apple News Post ID (channels)'),
    'group' => t('Apple News Article'),
    'field' => [
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
  ];

  $data['apple_news_channels']['channel_id'] = [
    'title' => t('Channel ID'),
    'help' => t('Apple News Channel ID (channels)'),
    'group' => t('Apple News Article'),
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

  // Sections table
  $data['apple_news_sections']['table']['join'] = [
    'apple_news_entities' => [
      'left_field' => 'post_id',
      'field' => 'post_id'
    ]
  ];

  $data['apple_news_sections']['post_id'] = [
    'title' => t('Post ID'),
    'help' => t('Apple News Post ID'),
    'group' => t('Apple News Article'),
    'field' => [
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ],
    'filter' => [
      'handler' => 'views_handler_filter_numeric',
    ],
    'sort' => [
      'handler' => 'views_handler_sort',
    ],
    'argument' => [
      'handler' => 'views_handler_argument_numeric',
    ],
  ];

  $data['apple_news_sections']['channel_id'] = [
    'title' => t('Channel ID'),
    'help' => t('Apple News Channel ID (sections)'),
    'group' => t('Apple News Article'),
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

  $data['apple_news_sections']['section_id'] = [
    'title' => t('Section ID'),
    'help' => t('Apple News Channel ID (sections)'),
    'group' => t('Apple News Article'),
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

  return $data;

}

/**
 * Implements hook_views_default_views().
 */
function apple_news_views_default_views() {
  // Default view
  $view = new view();
  $view->name = 'apple_news';
  $view->description = 'Apple News default view';
  $view->tag = 'Apple News';
  $view->base_table = 'apple_news_entities';
  $view->human_name = 'Apple News';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Published content';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'perm';
  $handler->display->display_options['access']['perm'] = 'administer apple news';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['submit_button'] = 'Search';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '25';
  $handler->display->display_options['style_plugin'] = 'table';
  $handler->display->display_options['style_options']['columns'] = array(
    'title' => 'title',
    'article_id' => 'article_id',
    'article_revision_id' => 'article_revision_id',
    'postdate' => 'postdate',
    'edit_node' => 'edit_node',
  );
  $handler->display->display_options['style_options']['default'] = 'postdate';
  $handler->display->display_options['style_options']['info'] = array(
    'title' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'article_id' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'article_revision_id' => array(
      'sortable' => 1,
      'default_sort_order' => 'asc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'postdate' => array(
      'sortable' => 1,
      'default_sort_order' => 'desc',
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
    'edit_node' => array(
      'align' => '',
      'separator' => '',
      'empty_column' => 0,
    ),
  );
  $handler->display->display_options['style_options']['empty_table'] = TRUE;
  /* No results behavior: Global: Unfiltered text */
  $handler->display->display_options['empty']['area_text_custom']['id'] = 'area_text_custom';
  $handler->display->display_options['empty']['area_text_custom']['table'] = 'views';
  $handler->display->display_options['empty']['area_text_custom']['field'] = 'area_text_custom';
  $handler->display->display_options['empty']['area_text_custom']['empty'] = TRUE;
  $handler->display->display_options['empty']['area_text_custom']['content'] = 'Currently there are no Apple News published content.';
  /* Relationship: Apple News Article: Apple News Referencing entity_id */
  $handler->display->display_options['relationships']['reverse_entity_id_node']['id'] = 'reverse_entity_id_node';
  $handler->display->display_options['relationships']['reverse_entity_id_node']['table'] = 'apple_news_entities';
  $handler->display->display_options['relationships']['reverse_entity_id_node']['field'] = 'reverse_entity_id_node';
  $handler->display->display_options['relationships']['reverse_entity_id_node']['required'] = TRUE;
  /* Field: Content: Title */
  $handler->display->display_options['fields']['title']['id'] = 'title';
  $handler->display->display_options['fields']['title']['table'] = 'node';
  $handler->display->display_options['fields']['title']['field'] = 'title';
  $handler->display->display_options['fields']['title']['relationship'] = 'reverse_entity_id_node';
  /* Field: Apple News Article: Post ID */
  $handler->display->display_options['fields']['post_id']['id'] = 'post_id';
  $handler->display->display_options['fields']['post_id']['table'] = 'apple_news_entities';
  $handler->display->display_options['fields']['post_id']['field'] = 'post_id';
  $handler->display->display_options['fields']['post_id']['label'] = '';
  $handler->display->display_options['fields']['post_id']['exclude'] = TRUE;
  $handler->display->display_options['fields']['post_id']['element_label_colon'] = FALSE;
  /* Field: Apple News Article: Share URL */
  $handler->display->display_options['fields']['share_url']['id'] = 'share_url';
  $handler->display->display_options['fields']['share_url']['table'] = 'apple_news_entities';
  $handler->display->display_options['fields']['share_url']['field'] = 'share_url';
  /* Field: Apple News Article: Post date */
  $handler->display->display_options['fields']['postdate']['id'] = 'postdate';
  $handler->display->display_options['fields']['postdate']['table'] = 'apple_news_entities';
  $handler->display->display_options['fields']['postdate']['field'] = 'postdate';
  $handler->display->display_options['fields']['postdate']['date_format'] = 'short';
  $handler->display->display_options['fields']['postdate']['second_date_format'] = 'long';
  /* Field: Content: Edit link */
  $handler->display->display_options['fields']['edit_node']['id'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['table'] = 'views_entity_node';
  $handler->display->display_options['fields']['edit_node']['field'] = 'edit_node';
  $handler->display->display_options['fields']['edit_node']['relationship'] = 'reverse_entity_id_node';
  $handler->display->display_options['fields']['edit_node']['label'] = '';
  $handler->display->display_options['fields']['edit_node']['element_label_colon'] = FALSE;
  /* Filter criterion: Content: Title */
  $handler->display->display_options['filters']['title']['id'] = 'title';
  $handler->display->display_options['filters']['title']['table'] = 'node';
  $handler->display->display_options['filters']['title']['field'] = 'title';
  $handler->display->display_options['filters']['title']['relationship'] = 'reverse_entity_id_node';
  $handler->display->display_options['filters']['title']['operator'] = 'contains';
  $handler->display->display_options['filters']['title']['exposed'] = TRUE;
  $handler->display->display_options['filters']['title']['expose']['operator_id'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['label'] = 'Title';
  $handler->display->display_options['filters']['title']['expose']['operator'] = 'title_op';
  $handler->display->display_options['filters']['title']['expose']['identifier'] = 'title';
  $handler->display->display_options['filters']['title']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );

  /* Display: Published content */
  $handler = $view->new_display('page', 'Published content', 'page');
  $handler->display->display_options['path'] = 'admin/config/content/apple-news/published';
  $handler->display->display_options['menu']['type'] = 'tab';
  $handler->display->display_options['menu']['title'] = 'Published content';
  $handler->display->display_options['menu']['weight'] = '0';
  $handler->display->display_options['menu']['context'] = 0;
  $handler->display->display_options['menu']['context_only_inline'] = 0;

  $views[$view->name] = $view;

  return $views;
}
