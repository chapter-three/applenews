<?php
/**
 * @file
 * Entity hook implementations.
 */

/**
 * Implements hook_entity_insert().
 */
function apple_news_entity_insert($entity, $type) {
  if (!empty($entity->apple_news['apple_news_publish_flag'])) {
    _apple_news_entity_push('insert', $entity, $type);
  }
}

/**
 * Implements hook_entity_update().
 */
function apple_news_entity_update($entity, $type) {
  if (!empty($entity->apple_news['apple_news_publish_flag'])) {
    _apple_news_entity_push('update', $entity, $type);
  }
}

/**
 * Push node.
 */
function _apple_news_entity_push($op, $entity, $type) {
  module_load_include('inc', 'apple_news');

  $wrapper = entity_metadata_wrapper($type, $entity);
  $export = apple_news_entity_get_export($type, $wrapper->getIdentifier());

  $el = $entity->apple_news;
  foreach (apple_news_channels(TRUE) as $channel_id => $channel) {
    if (!empty($el['channels']['channel-' . $channel_id])) {
      $settings = [];
      foreach ($channel['sections'] as $section_id => $section) {
        if (!empty($el['channels']['section-' . $channel_id . '-' . $section_id])) {
          $settings['metadata']['data']['links']['sections'][] = apple_news_section_url($section_id);
        }
      }
      $settings['override_revision'] = !empty($el['override_revision']);
      if (!apple_news_op($op, $channel_id, $entity, $type, $export, $settings)) {
        drupal_set_message(
          t('Unable to push %title to %channel',
            [
              '%title'   => $wrapper->label(),
              '%channel' => filter_xss($channel->name)
            ]), 'error'
        );
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
      if (!apple_news_op('delete', $channel_id, $entity, $type)) {
        $wrapper = entity_metadata_wrapper($type, $entity);
        $channel = apple_news_channel($channel_id);
        drupal_set_message(
          t('Unable to delete %title from %channel',
            [
              '%title'   => $wrapper->label(),
              '%channel' => filter_xss($channel->name)
            ]), 'error'
        );
      }
    }
  }
}
