<?php

/**
 * @file
 * Helper functions.
 */

/**
 * Defines an export unique key.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Export machine name.
 *
 * @return string
 *   Key.
 */
function _apple_news_export_id($module, $machine_name) {
  return "${module}::${machine_name}";
}

/**
 * Decodes an export id.
 *
 * @param string $id
 *   Key.
 *
 * @return array
 *   Module machine name, export machine name.
 */
function _apple_news_export_id_extract($id) {
  return explode('::', $id);
}

/**
 * Implements OBJECT_save().
 *
 * @param ApplenewsExport $export
 *   Object as returned by apple_news_export_load().
 *
 * @return bool
 *   Success.
 */
function apple_news_export_save(ApplenewsExport $export) {
  $transaction = db_transaction();
  $msg = t('Could not save export %machine_name defined by %module.');
  $var = [
    '%machine_name' => $export->getMachineName(),
    '%module'       => $export->getModule(),
  ];
  try {
    $update = !empty($export->id);
    if ($update) {
      db_update('apple_news_status')
        ->fields([
          'enabled'  => (int) $export->enabled,
          'instance' => serialize($export),
        ])
        ->condition('id', $export->id)
        ->execute();
    }
    else {
      $export->id = db_insert('apple_news_status')
        ->fields([
          'module'       => $export->getModule(),
          'machine_name' => $export->getMachineName(),
          'enabled'      => (int) $export->enabled,
          'instance'     => serialize($export),
        ])
        ->execute();
    }
    // @todo just need to flush cache.
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('apple_news', $e, $msg, $var, WATCHDOG_ERROR);
  }
  return FALSE;
}

/**
 * Implements OBJECT_delete().
 *
 * @param ApplenewsExport $export
 *   Object as returned by apple_news_export_load().
 *
 * @return bool
 *   Success.
 */
function apple_news_export_delete(ApplenewsExport $export) {
  $transaction = db_transaction();
  $msg = t('Could not delete export %id.');
  $var = [
    '%id' => $export->id,
  ];
  try {
    db_delete('apple_news_status')
      ->condition('id', $export->id)
      ->execute();
    db_delete('apple_news_log')
      ->condition('exid', $export->id)
      ->execute();
    db_update('apple_news_entities')
      ->fields(['exid' => NULL])
      ->condition('exid', $export->id)
      ->execute();
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('apple_news', $e, $msg, $var, WATCHDOG_ERROR);
  }
  return FALSE;
}

/**
 * Instantiates a new destination object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Export machine name.
 *
 * @return ApplenewsExport
 *   The instantiated class configured for the export.
 */
function apple_news_new_export($module, $machine_name) {
  return _apple_news_new('export', $module, $machine_name);
}

/**
 * Instantiates a new destination object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Destination machine name.
 *
 * @return ApplenewsDestination
 *   The instantiated class specified by the implementation..
 */
function apple_news_new_destination($module, $machine_name) {
  return _apple_news_new('destination', $module, $machine_name);
}

/**
 * Instantiates a new source object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Source machine name.
 *
 * @return ApplenewsSourceInterface
 *   The instantiated class specified by the implementation..
 */
function apple_news_new_source($module, $machine_name, $arg = NULL) {
  $implementations = apple_news_get_module_apis('source');
  $key = _apple_news_export_id($module, $machine_name);
  if (isset($implementations[$key])) {
    $info = $implementations[$key];
    $reflection = apple_news_validate_class($info['class'],
      'ApplenewsSourceInterface');
    if ($reflection) {
      $args = func_get_args();
      $args = array_splice($args, 2);
      return $reflection->newInstanceArgs($args);
    }
  }
  return NULL;
}

/**
 * Helper function for logging/validating of implementation-defined classes.
 *
 * @param string $class_name
 *   Class name.
 * @param string $is_a
 *   Optionally validate $class_name isa $is_a.
 *
 * @return ReflectionClass|bool
 *   Valid implementations return ReflectionClass, invalid FALSE.
 */
function apple_news_validate_class($class_name, $is_a = NULL) {
  if (!class_exists($class_name)) {
    $msg = 'Class %class does not exist.';
    $args = ['%class' => $class_name];
    watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
    return FALSE;
  }
  $reflection = new ReflectionClass($class_name);
  if ($reflection->isAbstract()) {
    $msg = 'Class %class is not a concrete class';
    $args = ['%class' => $class_name];
    watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
    return FALSE;
  }
  if ($is_a && !($class_name == $is_a || $reflection->isSubclassOf($is_a))) {
    $msg = 'Class %class is not a subclass of %isa';
    $args = ['%class' => $class_name, '%isa' => $is_a];
    watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
    return FALSE;
  }
  return $reflection;
}

/**
 * Gets an export object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Export machine name.
 *
 * @return ApplenewsExport
 *   The instantiated class configured for the export.
 *
 * @see apple_news_exports()
 */
function _apple_news_new($type, $module, $machine_name) {
  $key = _apple_news_export_id($module, $machine_name);

  $implementations = apple_news_get_module_apis($type);
  if (empty($implementations[$key])) {
    $msg = t('No !type %machine_name defined by %module.');
    $var = [
      '!type'         => $type,
      '%machine_name' => $machine_name,
      '%module'       => $module,
    ];
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }
  $info = $implementations[$key];
  if (!class_exists($info['class'])) {
    $msg = 'No !type class %class found.';
    $var = [
      '!type'  => $type,
      '%class' => $info['class']
    ];
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }

  // Instantiate.
  try {
    if (!isset($info['arguments']) || !is_array($info['arguments'])) {
      $info['arguments'] = [];
    }
    $object = new $info['class']($module, $machine_name, $info['arguments']);
    if (!empty($info['default']) && function_exists($info['default'])) {
      $info['default']($object);
    }
    return $object;
  }
  catch (Exception $e) {
    $msg = t('!type class %class could not be constructed: %message');
    $var = [
      '!type'    => $type,
      '%class'   => $info['class'],
      '%message' => $e->getMessage(),
    ];
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
  }
  return NULL;
}

/**
 * Gets all exports.
 *
 * @param bool $reset
 *   Force fetch from db.
 *
 * @return array
 *   Associative array keyed on export id of info arrays as defined by
 *   hook_apple_news_api(), with 2 additional keys:
 *   - #id -- Export ID.
 *   - #enabled -- Export can be run.
 */
function apple_news_exports($reset = NULL) {
  $exports = &drupal_static(__FUNCTION__);
  if (!empty($exports) && empty($reset)) {
    return $exports;
  }

  $implementations = apple_news_get_module_apis('export');

  $exports = [];
  $result = db_select('apple_news_status', 's')
    ->fields('s')
    ->orderBy('module')
    ->orderBy('machine_name')
    ->execute();
  foreach ($result as $row) {
    // Module still enabled and export still defined.
    $key = _apple_news_export_id($row->module, $row->machine_name);
    if (!isset($implementations[$key])) {
      $msg = 'Export %machine_name defined by %module no longer defined.';
      $args = [
        '%machine_name' => $row->machine_name,
        '%module'       => $row->module,
      ];
      watchdog('apple_news', $msg, $args, WATCHDOG_INFO);
      continue;
    }
    // Good to go.
    $exports[$row->id]['#id'] = $row->id;
    $exports[$row->id]['#enabled'] = !empty($row->enabled) ? $row->enabled : NULL;
  }
  return $exports;
}

/**
 * Gets registered and enabled exports.
 *
 * @param bool $reset
 *   Force fetch from db.
 *
 * @return array
 *   Associative array keyed on export id of info arrays as defined by
 *   hook_apple_news_api(), with 2 additional keys:
 *   - #id -- Export ID.
 *   - #enabled -- Export can be run.
 */
function apple_news_exports_enabled($reset = NULL) {
  return array_filter(apple_news_exports($reset), function($info) {
    return !empty($info['#enabled']);
  });
}

/**
 * Get a list of implementations supporting the current API.
 *
 * @param string $class
 *   One of:
 *   - "export".
 *   - "destination".
 *   - "source".
 * @param string $type
 *   If $class is "destination", one of:
 *   - "component".
 * @param bool $reset
 *   Force fetch from db.
 *
 * @return array
 *   List of implementation info arrays. If $class is NULL, the arrays are
 *   nested in an associative array keyed on $class.
 *
 * @see hook_apple_news_api()
 */
function apple_news_get_module_apis($class = NULL, $type = NULL, $reset = FALSE) {
  $implementations = &drupal_static(__FUNCTION__);
  if (empty($implementations) || $reset) {
    $implementations = [];

    $validate = function($type, $module, $machine_name, $definition) {
      $args = [
        '!type'         => $type,
        '%machine_name' => $machine_name,
        '%module'       => $module,
      ];
      if (empty($definition['class'])) {
        $msg = '!type %machine_name defined by %module is missing required class';
        watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
        return FALSE;
      }
      if (!preg_match('/^[a-zA-Z0-9-_]+$/', $machine_name)) {
        $msg = '!type machine name "%machine_name" defined by %module must match /^[a-zA-Z0-9-_]$/';
        watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
        return FALSE;
      }
      return TRUE;
    };

    foreach (module_implements('apple_news_api') as $module) {
      $function = $module . '_apple_news_api';
      $info = $function() + [
          'exports'      => [],
          'sources'      => [],
          'destinations' => [],
        ];

      // Check API version.
      if (!isset($info['api']) || $info['api'] != APPLE_NEWS_API_VERSION) {
        $msg = t('%function supports Apple News API version %modversion, Apple News module API version is %version - migration support not loaded.');
        $args = [
          '%function'   => $function,
          '%modversion' => $info['api'],
          '%version'    => APPLE_NEWS_API_VERSION,
        ];
        watchdog('apple_news', $msg, $args, WATCHDOG_NOTICE);
        continue;
      }

      // Exports.
      foreach ($info['exports'] as $machine_name => $export) {
        if (!$validate('Export', $module, $machine_name, $export)) {
          continue;
        }
        $key = _apple_news_export_id($module, $machine_name);
        $implementations['export'][$key] = array_merge([
          'name'        => '',
          'description' => '',
          'arguments'   => [],
        ],
          $export,
          [
            'module'       => $module,
            'machine_name' => $machine_name,
          ]
        );
      }

      // Source classes.
      foreach ($info['sources'] as $machine_name => $source) {
        if (!$validate('Source', $module, $machine_name, $source)) {
          continue;
        }
        $key = _apple_news_export_id($module, $machine_name);
        // Set "type" to classify source -- each type will take different
        // different parameters in its constructor.
        // @see ApplenewsDestination::getSource()
        $reflection = apple_news_validate_class($source['class']);
        if ($source['class'] == 'ApplenewsSourceValue' ||
          $reflection->isSubclassOf('ApplenewsSourceValue')
        ) {
          $class_type = 'value';
        }
        elseif ($source['class'] == 'ApplenewsSourceEntity' ||
          $reflection->isSubclassOf('ApplenewsSourceEntity')
        ) {
          $class_type = 'entity';
        }
        elseif ($source['class'] == 'ApplenewsSourceField' ||
          $reflection->isSubclassOf('ApplenewsSourceField')
        ) {
          $class_type = 'field';
        }
        else {
          $msg = 'Class %class is not a subclass of ApplenewsSourceValue, ApplenewsSourceEntity or ApplenewsSourceField.';
          $args = ['%class' => $source['class']];
          watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
          return FALSE;
        }
        $implementations['source'][$key] = array_merge([
          'name'        => '',
          'description' => '',
        ],
          $source,
          [
            'module'       => $module,
            'machine_name' => $machine_name,
            'type'         => $class_type,
          ]
        );
      }

      // Destination classes.
      foreach ($info['destinations'] as $machine_name => $destination) {
        if (!$validate('Destination', $module, $machine_name, $destination)) {
          continue;
        }
        $key = _apple_news_export_id($module, $machine_name);
        // Set "type" to classify destination.
        // @see ApplenewsExportConfigurable::getConfigForm()
        $reflection = apple_news_validate_class($destination['class'],
          'ApplenewsDestination');
        if ($destination['class'] == 'ApplenewsDestinationComponent' ||
          $reflection->isSubclassOf('ApplenewsDestinationComponent')
        ) {
          $class_type = 'component';
        }
        else {
          $msg = 'Class %class is not a subclass of ApplenewsDestinationComponent.';
          $args = ['%class' => $destination['class']];
          watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
          return FALSE;
        }
        $implementations['destination'][$key] = array_merge([
          'name'        => '',
          'description' => '',
        ],
          $destination, [
            'module'       => $module,
            'machine_name' => $machine_name,
            'type'         => $class_type,
          ]
        );
      }

    }

    // Allow modules to alter the migration information.
    drupal_alter('apple_news_api', $implementations);
  }

  if (empty($class)) {
    return $implementations;
  }

  if (!in_array($class, ['export', 'destination', 'source'])) {
    watchdog('apple_news', 'Invalid $class "%class".', ['%class' => $class], WATCHDOG_ERROR);
    return NULL;
  }
  if ($class != 'destination' || empty($type)) {
    return $implementations[$class];
  }

  if (!in_array($type, ['component'])) {
    watchdog('apple_news', 'Invalid $type "%type".', ['%type' => $type], WATCHDOG_ERROR);
    return NULL;
  }
  return array_filter($implementations[$class], function($info) use($type) {
    return $info['type'] == $type;
  });
}

/**
 * Find export for an entity.
 *
 * @return ApplenewsExport|NULL
 *   The configured export object, or NULL if none configured.
 */
function apple_news_entity_get_export($entity_type, $entity_id) {
  // Has the entity been assigned an export?
  $post = new ApplenewsDatabase();
  $post->setPostId($entity_type, $entity_id, $entity_id);
  if (($article = $post->getArticle()) ||
      !empty($article['exid'])
  ) {
    if ($export = apple_news_export_load($article['exid'])) {
      return $export;
    }
  }
  // Grab first matching export.
  foreach (apple_news_exports_enabled() as $id => $info) {
    $export = apple_news_export_load($id);
    if (apple_news_entity_export_valid($export, $entity_type, $entity_id)) {
      return $export;
    }
  }
  // No matching export.
  return NULL;
}

/**
 * Determine if an export is valid for an entity.
 */
function apple_news_entity_export_valid(ApplenewsExport $export, $entity_type, $entity_id) {
  $query = $export->query();
  $query->entityCondition('entity_id', $entity_id);
  $result = $query->execute();
  return isset($result[$entity_type][$entity_id]);
}

/**
 * Export an individual drupal entity.
 *
 * @param ApplenewsExport $export
 *   Export to test.
 * @param string $entity_type
 *   Entity type.
 * @param string $entity_id
 *   Entity ID.
 *
 * @return array
 *   With keys:
 *   - json: The apple news document JSON.
 *   - files: Array of asset paths keyed on Apple Document URL.
 */
function apple_news_entity_export(ApplenewsExport $export, $entity_type, $entity_id) {
  $entity = entity_load_single($entity_type, $entity_id);
  return [
    'json'  => $export->setSource($entity)->export()->json(),
    'files' => $export->getAssets(),
  ];
}

/**
 * Set export status.
 */
function apple_news_batch_export_set_status($name, $info) {
  $status = variable_get('apple_news_batch_export_status', []);
  $status[$name] = $info;
  variable_set('apple_news_batch_export_status', $status);
}

/**
 * Get export status.
 */
function apple_news_batch_export_get_status($batch_name) {
  $status = variable_get('apple_news_batch_export_status', []);
  if (!empty($status)) {
    foreach ($status as $name => $info) {
      if ($name == $batch_name) {
        return $info;
      }
    }
  }
  return [];
}

/**
 * Post Article to Apple News.
 *
 * @todo Add Revision support.
 *
 * @param string $channel_id
 *   News channel id.
 * @param string $entity_type
 *   Drupal entity type.
 * @param \EntityDrupalWrapper $entity_wrapper
 *   Entity metadata wrapper for the entity to post.
 * @param array $data
 *   Associative array with keys:
 *   - json - News document json.
 *   - files - Bundled files.
 *   - metadata - News metadata.
 *
 * @return bool
 *   Success.
 */
function apple_news_post($channel_id, $entity_type, EntityDrupalWrapper $entity_wrapper, array $data) {
  module_load_include('inc', 'apple_news', 'apple_news.publisher_api');
  // Make POST request to Apple News via PublisherAPI.
  $response = apple_news_post_article($channel_id, $data);
  // If successful request.
  if (!isset($response->errors) && isset($response->data->id)) {
    // Invoke Apple News insert hooks.
    module_invoke_all('apple_news_insert', $response->data->id, $response->data->revision, $channel_id, $entity_wrapper, $entity_type, $data);
    $sections = [];
    foreach ($response->data->links->sections as $section_url) {
      $c = explode("/", $section_url);
      $sections[] = [
        'channel_id' => $channel_id,
        'section_id' => $c[count($c) - 1],
      ];
    }
    $record = [
      'entity'   => [
        'article_id'          => $response->data->id,
        'article_revision_id' => $response->data->revision,
        'share_url'           => $response->data->shareUrl,
        'postdate'            => REQUEST_TIME,
      ],
      'channels' => [
        [
          'channel_id' => $channel_id,
        ]
      ],
      'sections' => $sections,
      'metadata' => [
        'data' =>
        [
          'shareUrl'     => !empty($response->data->shareUrl) ? $response->data->shareUrl : '',
          'type'         => !empty($response->data->type) ? $response->data->type : '',
          'links'        => !empty($response->data->links) ? $response->data->links : '',
          'isSponsored'  => !empty($response->data->isSponsored) ? $response->data->isSponsored : '',
        ]
      ],
    ];
    $post = new ApplenewsDatabase();
    $post->setEntityId($entity_wrapper->getIdentifier());
    $post->setEntityRevisionId($entity_wrapper->getIdentifier());
    $post->setEntityType($entity_type);
    $post->insert($record);
    return TRUE;
  }
  else {
    watchdog('apple_news', "Could not post an Apple News article @id",
      ['@id' => $entity_wrapper->getIdentifier()],
      WATCHDOG_NOTICE
    );
  }
  return FALSE;
}

/**
 * Post Article to Apple News.
 *
 * @todo Add Revision support.
 *
 * @param string $channel_id
 *   News channel id.
 * @param string $entity_type
 *   Drupal entity type.
 * @param \EntityDrupalWrapper $entity_wrapper
 *   Entity metadata wrapper for the entity to post.
 * @param array $data
 *   Associative array with keys:
 *   - json - News document json.
 *   - files - Bundled files.
 *   - metadata - News metadata.
 *
 * @return bool
 *   Success.
 */
function apple_news_update($channel_id, $entity_type, EntityDrupalWrapper $entity_wrapper, array $data) {
  module_load_include('inc', 'apple_news', 'apple_news.publisher_api');

  $query = db_select('apple_news_entities', 'e');
  $query->join('apple_news_channels', 'c', 'c.post_id = e.post_id');
  $query->fields('e', ['article_id', 'article_revision_id']);
  $query->fields('c', ['channel_id']);
  $query->condition('e.entity_type', $entity_type);
  $query->condition('e.entity_id', $entity_wrapper->getIdentifier());
  $query->condition('c.channel_id', $channel_id);
  $article = $query->execute()->fetchObject();

  // Make sure article exists.
  if (!empty($article->article_id)) {

    // Add metadata.
    if (!empty($data['metadata'])) {
      $data['metadata'] = json_decode($data['metadata'], TRUE);
    }

    // Apple News Revision ID. Required in order to be able to update articles.
    $apple_news_revision_id = NULL;
    if ($apple_news_article = apple_news_get_article($article->article_id)) {
      // This will override previous revisions.
      $apple_news_revision_id = $apple_news_article->data->revision;
    }
    /*
    else {
      $apple_news_revision_id = $article->article_revision_id;
    }
    */

    $data['metadata']['data']['revision'] = $apple_news_revision_id;
    $data['metadata'] = json_encode($data['metadata']);
    // Make POST request to Apple News via PublisherAPI.
    $response = apple_news_update_article($article->article_id, $data);
    // If successful request.
    if (!isset($response->errors) && isset($response->data->id)) {
      // Invoke Apple News update hooks.
      module_invoke_all('apple_news_update', $article->article_id, $article->article_revision_id, $channel_id, $entity_wrapper, $entity_type, $data);
      $sections = [];
      foreach ($response->data->links->sections as $section_url) {
        $c = explode("/", $section_url);
        $sections[] = [
          'channel_id' => $channel_id,
          'section_id' => $c[count($c) - 1],
        ];
      }
      $record = [
        'entity'   => [
          'article_id'          => $response->data->id,
          'article_revision_id' => $response->data->revision,
          'share_url'           => $response->data->shareUrl,
          'postdate'            => REQUEST_TIME,
        ],
        'channels' => [
          [
            'channel_id' => $channel_id,
          ]
        ],
        'sections' => $sections,
        'metadata' => [
          'data' =>
          [
            'shareUrl'     => !empty($response->data->shareUrl) ? $response->data->shareUrl : '',
            'type'         => !empty($response->data->type) ? $response->data->type : '',
            'links'        => !empty($response->data->links) ? $response->data->links : '',
            'isSponsored'  => !empty($response->data->isSponsored) ? $response->data->isSponsored : '',
          ]
        ],
      ];
      $post = new ApplenewsDatabase();
      $post->setPostId($entity_type, $entity_wrapper->getIdentifier(), $entity_wrapper->getIdentifier());
      $post->update($record);
      return TRUE;
    }
    else {
      watchdog('apple_news', "Could not update Apple News article @id (entity ID: @eid). You could try delete the article from Apple News and re-publish it.",
        [
          '@id'   => $article->article_id,
          '@eid' => $entity_wrapper->getIdentifier(),
        ],
        WATCHDOG_NOTICE
      );
    }
  }
  else {
    // Create new Apple News article.
    return apple_news_post($channel_id, $entity_type, $entity_wrapper, $data);
  }
  return FALSE;
}

/**
 * Delete Article from Apple News.
 *
 * @param string $channel_id
 *   News channel id.
 * @param string $entity_type
 *   Drupal entity type.
 * @param \EntityDrupalWrapper $entity_wrapper
 *   Entity metadata wrapper for the entity to post.
 *
 * @return bool
 *   Success.
 */
function apple_news_delete($channel_id, $entity_type, EntityDrupalWrapper $entity_wrapper) {
  module_load_include('inc', 'apple_news', 'apple_news.publisher_api');

  // Get article ID.
  $article_id = db_select('apple_news_entities', 'e')
    ->fields('e', ['article_id'])
    ->condition('entity_type', $entity_type)
    ->condition('entity_id', $entity_wrapper->getIdentifier())
    ->execute()
    ->fetchField();

  // Make DELETE request to Apple News via PublisherAPI.
  $response = apple_news_delete_article($article_id);
  // If successful request.
  if (empty($response)) {
    // Invoke Apple News delete hooks.
    module_invoke_all('apple_news_delete', $article_id, $channel_id, $entity_wrapper, $entity_type);
    $post = new ApplenewsDatabase();
    $post->setPostId($entity_type, $entity_wrapper->getIdentifier(), $entity_wrapper->getIdentifier());
    $post->delete();
    // $post->deleteByEntity($entity_wrapper, $entity_type);
    watchdog('apple_news', "Entity ID @id was removed from Apple News",
      ['@id' => $entity_wrapper->getIdentifier()],
      WATCHDOG_INFO
    );
    return TRUE;
  }
  return FALSE;
}

/**
 * Perform operations on an entity.
 *
 * @param string $operation
 *   Entity operation (insert/update/delete).
 * @param string $channel_id
 *   Apple News channel ID.
 * @param object $entity
 *   Entity object.
 * @param string $entity_type
 *   Entity type.
 * @param ApplenewsExport $export
 *   Export object.
 *
 * @return bool
 *   Success.
 */
function apple_news_op($operation, $channel_id, $entity, $entity_type, ApplenewsExport $export = NULL) {
  $result = FALSE;
  /** @var EntityDrupalWrapper $entity_wrapper */
  $entity_wrapper = entity_metadata_wrapper($entity_type, $entity);
  if ($operation != 'delete') {
    // Generate Apple News Document formatted JSON string.
    $document = $export->setSource($entity)->export();
    $data = [
      'json'              => $document->json(),
      'files'             => $export->getAssets(),
      'metadata'          => !empty($settings['metadata']) ? json_encode($settings['metadata']) : '',
    ];
  }

  switch ($operation) {

    case 'insert':
      // Create new article in Apple News.
      $result = apple_news_post($channel_id, $entity_type, $entity_wrapper, $data);
      break;

    case 'update':
      // Update existing Apple News article.
      $result = apple_news_update($channel_id, $entity_type, $entity_wrapper, $data);
      break;

    case 'delete':
      // Delete article from Apple News.
      $result = apple_news_delete($channel_id, $entity_type, $entity_wrapper);
      break;
  }
  return $result;
}

/**
 * Converts drupal language code to appropriate Apples News value.
 *
 * @param string $language_code
 *   Drupal language code.
 *
 * @return string
 *   Apple news document language code.
 */
function apple_news_language_code($language_code) {
  // @todo
  return $language_code;
}

/**
 * Form configuration fields.
 */
function apple_news_settings_fields() {
  $config_fields = [
    'apple_news_variable_api_endpoint',
    'apple_news_variable_api_key',
    'apple_news_variable_api_secret',
    'apple_news_variable_api_channels',
  ];
  return $config_fields;
}

/**
 * Get Apple News channels.
 */
function apple_news_channels($grouped = FALSE) {
  // Add Post to Apple News checkbox.
  $channels = [];
  $api_channels = variable_get('apple_news_variable_api_channels', []);
  if (empty($api_channels)) {
    return [];
  }
  if ($grouped) {
    foreach ($api_channels as $channel_id) {
      $sections = [];
      $channel = apple_news_channel($channel_id);
      $api_sections = apple_news_sections($channel->id);
      foreach ($api_sections as $section) {
        $sections[$section->id] = [
          'name'    => filter_xss($section->name),
          'url'     => $section->links->self,
          'default' => $section->isDefault,
        ];
      }
      $channels[$channel->id] = [
        'name'     => filter_xss($channel->name),
        'sections' => $sections,
      ];
    }
  }
  else {
    foreach ($api_channels as $channel_id) {
      $channel = apple_news_channel($channel_id);
      $channels[$channel->id] = filter_xss($channel->name);
    }
  }
  return $channels;
}

/**
 * Remove data related to $channel_id.
 *
 * Use this when removing channel ID from settings.
 *
 * @see apple_news.admin.inc
 */
function apple_news_cleanup_data(stdClass $channel) {

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
    $enabled = $row->enabled;
    $instance = unserialize($row->instance);
    $channels_all = apple_news_channels(TRUE);
    if (isset($channels_all[$channel->id])) {
      unset($instance->settings['channels'][$channel->id]);
      foreach ($channels_all[$channel->id]['sections'] as $section_id => $section) {
        unset($instance->settings['sections'][$section_id]);
      }
    }
    // Automatically disable export when no channels available.
    if (empty($instance->settings['channels'])) {
      $enabled = 0;
      $instance->enabled = $enabled;
    }
    db_update('apple_news_status')
      ->fields(
        [
          'enabled'  => $enabled,
          'instance' => serialize($instance),
        ]
      )
      ->condition('module', $row->module)
      ->condition('machine_name', $row->machine_name)
      ->execute();

    $channels = variable_get('apple_news_variable_api_channels', []);
    foreach ($channels as $i => $channel_id) {
      if ($channel_id == $channel->id) {
        unset($channels[$i]);
      }
    }
    variable_set('apple_news_variable_api_channels', $channels);
  }
}

/**
 * Get Apple News sections.
 *
 * @see apple_news_get_sections()
 */
function apple_news_sections($channel_id) {
  $sections = [];
  if ($cache = cache_get('sections_' . $channel_id, 'cache_apple_news')) {
    $sections = $cache->data;
  }
  else {
    module_load_include('inc', 'apple_news', 'apple_news.publisher_api');
    $response = apple_news_get_sections($channel_id);
    if (!empty($response->data)) {
      foreach ($response->data as $section) {
        $sections[] = $section;
      }
    }
    cache_set('sections_' . $channel_id, $sections, 'cache_apple_news');
  }
  return $sections;
}

/**
 * Get Apple News section information.
 */
function apple_news_section($section_id) {
  $sections = apple_news_channels(TRUE);
  foreach ($sections as $channel_id => $channel) {
    if (!empty($channel['sections'][$section_id])) {
      return $channel['sections'][$section_id];
    }
  }
  return FALSE;
}

/**
 * Get Apple News channels.
 *
 * @see apple_news_get_sections()
 */
function apple_news_channel($channel_id) {
  $channel = NULL;
  if ($cache = cache_get('channel_' . $channel_id, 'cache_apple_news')) {
    $channel = $cache->data;
  }
  else {
    module_load_include('inc', 'apple_news', 'apple_news.publisher_api');
    $response = apple_news_get_channel($channel_id);
    if (!isset($response->errors)) {
      if (!empty($response->data)) {
        $channel = $response->data;
        cache_set('channel_' . $channel_id, $channel, 'cache_apple_news');
      }
    }
    else {
      return $response;
    }
  }
  return $channel;
}

/**
 * Load entity channels.
 *
 * @param object $entity
 *   Entity object.
 * @param string $type
 *   Entity type.
 *
 * @return object
 *   Entity objects.
 */
function apple_news_load_article_channels($entity, $type) {
  $post = new ApplenewsDatabase();
  /** @var EntityDrupalWrapper $entity_wrapper */
  $entity_wrapper = entity_metadata_wrapper($type, $entity);
  $post->setPostId($type, $entity_wrapper->getIdentifier(), $entity_wrapper->getIdentifier());
  $channels_all = apple_news_channels();
  $channel_ids = array_keys($channels_all);
  $post->setChannelId($channel_ids);
  $channels = $post->getChannels();
  if (!empty($channels)) {
    return $channels;
  }
  return FALSE;
}

/**
 * Check if the entity already published to Apple News.
 *
 * @param string $channel_id
 *   Apple News channel ID.
 * @param object $entity_wrapper
 *   Entity wrapper object.
 * @param string $entity_type
 *   Entity type.
 *
 * @return object|NULL
 *   Export log record or FALSE if not results.
 */
function apple_news_is_exported($channel_id, $entity_wrapper, $entity_type) {
  $post = new ApplenewsDatabase();
  $post->setPostId($entity_type, $entity_wrapper->getIdentifier(), $entity_wrapper->getIdentifier());
  $post->setChannelId($channel_id);
  $article = $post->getArticle();
  if (!empty($article['article_id'])) {
    return $article;
  }
  return FALSE;
}

/**
 * Get section URL.
 *
 * @param string $section_id
 *   Apple news section id.
 *
 * @return string
 *   URL
 */
function apple_news_section_url($section_id) {
  $url = variable_get('apple_news_variable_api_endpoint', FALSE);
  if ($url) {
    $url .= '/sections/' . $section_id;
  }
  return $url;
}

/**
 * Export to the Apple News preview-able format documents.
 */
function apple_news_export_to_file($entity_id, $entity_ids = [], $filename = NULL, $entity_archive = FALSE, $data = []) {
  $preview = new ApplenewsPreview($entity_id, $filename, $entity_archive, $data);
  if (!empty($entity_id)) {
    $preview->toFile();
    $preview->archive($entity_ids);
    return NULL;
  }
  else {
    $file_url = $preview->getArchiveFilePath();
    $preview->archive($entity_ids);
    return $file_url;
  }
}

/**
 * Get file archive path.
 */
function apple_news_export_file_path($entity_id) {
  $preview = new ApplenewsPreview(NULL, '', FALSE, []);
  return $preview->getEntityArchivePath($entity_id);
}

/**
 * Get file archive path.
 */
function apple_news_export_file_delete($entity_id) {
  $preview = new ApplenewsPreview(NULL, '', FALSE, []);
  $preview->entityArchiveDelete($entity_id);
}

/**
 * Remove all temporary files.
 */
function apple_news_export_dir_cleanup($entity_ids = []) {
  $preview = new ApplenewsPreview(NULL, '', FALSE, []);
  $preview->removeDirectories($entity_ids);
}

/**
 * Check if all required libraries installed.
 */
function apple_news_libraries_installed() {

  $libraries = [
    'php-curl-class' => CURL_SUPPORTED_VERSION,
    'AppleNews'      => APPLE_NEWS_SUPPORTED_VERSION
  ];

  $count_installed = 0;

  foreach ($libraries as $name => $version) {
    if (($library = libraries_detect($name)) &&
        !empty($library['installed']) &&
        $library['version'] == $version
    ) {
      $count_installed++;
    }
  }

  if ($count_installed != count($libraries)) {
    return [
      'info' => [
        '#markup' => t('Please install all required PHP libraries. See !link file for instructions.',
          [
            '!link' => l(t('README'), drupal_get_path('module', 'apple_news') . '/README.md', ['attributes' => ['target' => '_blank']]),
          ]
        ),
      ]
    ];
  }

  return [];

}

/**
 * Library autoinstall function (downloads and unzips library files).
 */
function apple_news_autoinstall_libraries() {

  $libraries = [
    'AppleNews' => [
      'version' => APPLE_NEWS_SUPPORTED_VERSION,
      'path'    => 'sites/all/libraries/AppleNews',
    ],
    'php-curl-class' => [
      'version' => CURL_SUPPORTED_VERSION,
      'path'    => 'sites/all/libraries/php-curl-class',
    ],
  ];

  foreach ($libraries as $name => $info) {
    if ($library = libraries_load($name)) {
      $lib_path = $info['path'];
      $lib_all_realpath = DRUPAL_ROOT . '/sites/all/libraries';
      $lib_realpath = DRUPAL_ROOT . '/' . $lib_path;
      $lib_version_realpath = $lib_realpath . '-' . $info['version'];
      $download_url = $library['download url'];

      // Get remote library file.
      $request = drupal_http_request($download_url);
      // If file exists.
      if ($request->code == 200) {
        if (!is_dir($lib_version_realpath)) {
          // Get archive basename.
          $filename = drupal_basename($download_url);
          $filepath = $lib_all_realpath . '/' . $filename;
          // Save remote archive file locally.
          file_unmanaged_save_data($request->data, $filepath);
          // Unzip the file.
          $zip = new \ZipArchive();
          if ($zip->open($filepath) === TRUE) {
            // Extract library archive file.
            $zip->extractTo($lib_all_realpath);
            // Delete the archive file.
            file_unmanaged_delete_recursive($lib_all_realpath . '/' . $filename);
            // Rename unzipped directory to match the lib path.
            if (!rename($lib_version_realpath, $lib_realpath)) {
              // Remove original PHP library directory.
              file_unmanaged_delete_recursive($lib_version_realpath);
            }
            // Display message in command line.
            if (drupal_is_cli() && function_exists('drush_log')) {
              drush_log($library['name'] . ' successfully downloaded and unzipped.', 'ok');
            }
          }
          else {
            // Display message in command line.
            if (drupal_is_cli() && function_exists('drush_log')) {
              drush_log('Failed to unzip ' . $library['name'] . '.', 'warning');
            }
          }
          $zip->close();
        }
      }
    }
  }
}
