<?php
/**
 * @file
 * Entity hook implementations.
 */

/**
 * Implements hook_entity_insert().
 */
function apple_news_entity_insert($entity, $type) {
  if (!empty($entity->apple_news_publish_flag)) {
    module_load_include('inc', 'apple_news');
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
                $settings['metadata']['data']['links']['sections'][] = apple_news_section_url($section_id);
              }
            }
            apple_news_op('insert', $channel_id, $entity, $type, $export, $settings);
          }
        }
      }
    }
  }
}

/**
 * Implements hook_entity_update().
 */
function apple_news_entity_update($entity, $type) {
  if (!empty($entity->apple_news_publish_flag)) {
    module_load_include('inc', 'apple_news');
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
                $settings['metadata']['data']['links']['sections'][] = apple_news_section_url($section_id);
              }
            }
            $settings['override_revision'] = !empty($entity->apple_news_override_revision) ? TRUE : FALSE;
            apple_news_op('update', $channel_id, $entity, $type, $export, $settings);
          }
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
  if ($channel_ids = apple_news_load_article_channels($entity, $type)) {
    foreach ($channel_ids as $channel_id) {
      apple_news_op('delete', $channel_id, $entity, $type);
    }
  }
}
