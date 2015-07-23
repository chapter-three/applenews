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
  if (!empty($entity->post_to_apple_news_channels)) {
    $existing = apple_news_load_posted_entity($entity, $type);
    foreach ($entity->post_to_apple_news_channels as $channel_id) {

    }
  }
}

/**
 * Implements hook_entity_update().
 */
function apple_news_entity_update($entity, $type) {
  module_load_include('inc', 'apple_news');
  if (!empty($entity->post_to_apple_news_channels)) {
    //$existing = apple_news_load_posted_entity($entity, $type);
    //print_r($entity->post_to_apple_news_channels);exit;
  }
}

/**
 * Implements hook_entity_delete().
 */
function apple_news_entity_delete($entity, $type) {
  module_load_include('inc', 'apple_news');
  //print_r($entity);exit;
}
