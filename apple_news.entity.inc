<?php
/**
 * @file
 * Entity hook implementations.
 */

/**
 * Implements hook_entity_insert().
 */
function apple_news_entity_insert($entity, $type) {
  module_load_include('inc', 'apple_news');
  module_load_include('inc', 'apple_news', 'push_api');
  foreach (apple_news_get_entity_types() as $module => $info) {
    if (!$info['enabled']) {
      continue;
    }
    $export = $info['#export'];
    $wrapper = entity_metadata_wrapper($type, $entity);
    if (in_array($wrapper->getBundle(), $export->bundles()) && in_array($type, $export->entityTypes())) {
      foreach (apple_news_channels(TRUE) as $channel_id => $channel) {
        if (!empty($entity->{'channel-' . $channel_id})) {
          $settings = [];
          foreach ($channel['sections'] as $section_id => $section) {
            if (!empty($entity->{'section-' . $channel_id . '-' . $section_id})) {
              $settings['metadata']['data']['links']['sections'][] = apple_news_push_api_section_url($section_id);
            }
          }
          apple_news_op('insert', $channel_id, $entity, $type, $export, $settings);
        }
      }
    }
  }
}

/**
 * Implements hook_entity_update().
 */
function apple_news_entity_update($entity, $type) {
  module_load_include('inc', 'apple_news');
  module_load_include('inc', 'apple_news', 'push_api');
  foreach (apple_news_get_entity_types() as $module => $info) {
    if (!$info['enabled']) {
      continue;
    }
    $export = $info['#export'];
    $wrapper = entity_metadata_wrapper($type, $entity);
    if (in_array($wrapper->getBundle(), $export->bundles()) && in_array($type, $export->entityTypes()) && $info['enabled']) {
      foreach (apple_news_channels(TRUE) as $channel_id => $channel) {
        if (!empty($entity->{'channel-' . $channel_id})) {
          $settings = [];
          foreach ($channel['sections'] as $section_id => $section) {
            if (!empty($entity->{'section-' . $channel_id . '-' . $section_id})) {
              $settings['metadata']['data']['links']['sections'][] = apple_news_push_api_section_url($section_id);
            }
          }
          apple_news_op('update', $channel_id, $entity, $type, $export, $settings);
        }
      }
    }
  }
}

/**
 * Implements hook_entity_delete().
 */
function apple_news_entity_delete($entity, $type) {
  module_load_include('inc', 'apple_news');
  $entities = apple_news_load_articles($entity, $type);
  foreach ($entities as $info) {
    apple_news_op('delete', $info->channel_id, $entity, $type);
  }
}
