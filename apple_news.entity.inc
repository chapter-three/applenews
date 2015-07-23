<?php
/**
 * @file
 * Entity hook implementations.
 */

/**
 * Implements hook_entity_insert().
 */
function apple_news_entity_insert($entity, $type) {
  if (!empty($entity->post_to_apple_news_channels)) {
  	module_load_include('inc', 'apple_news');
    foreach ($entity->post_to_apple_news_channels as $channel_id) {
      apple_news_op('insert', $channel_id, $entity, $type);
    }
  }
}

/**
 * Implements hook_entity_update().
 */
function apple_news_entity_update($entity, $type) {
  if (!empty($entity->post_to_apple_news_channels)) {
  	module_load_include('inc', 'apple_news');
    foreach ($entity->post_to_apple_news_channels as $channel_id) {
      apple_news_op('update', $channel_id, $entity, $type);
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
