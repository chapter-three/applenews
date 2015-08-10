<?php

/**
 * @file
 * Apple News DB class.
 *
 * This is a helper class to populate various Apple News
 * related database tables.
 */

namespace ChapterThree\AppleNews\Database;


interface ApplenewsDatabaseInterface {

  public function getArticle();

  public function setEntityId($id);
  public function setEntityRevisionId($id);
  public function setEntityType($type);
  public function setChannelId($id);

  public function setPostId($entity_type, $entity_id, $revision_id);
  public function setArticleId($article_id);

  public function insert($record);
  public function update($record);
  public function delete();

}

/**
 * Main class to work with AppleNews local database tables.
 */
class ApplenewsDatabase implements ApplenewsDatabaseInterface {

  private $post_id = NULL;
  private $entity_type = '';
  private $entity_id = NULL;
  private $revision_id = NULL;
  private $channel_id = NULL;

  /**
   * Save Apple News post information.
   */
  private function save_entities(array $entity = [], $primary_key = '') {
    if (!empty($primary_key)) {
      $entity['post_id'] = $this->setPostId($this->entity_type, $this->entity_id,  $this->revision_id);
    }
    $entity['entity_type'] = $this->entity_type;
    $entity['entity_id'] = $this->entity_id;
    $entity['revision_id'] = $this->revision_id;
    if (!empty($primary_key)) {
      drupal_write_record('apple_news_entities', $entity, $primary_key);
    }
    else {
      drupal_write_record('apple_news_entities', $entity);
    }
    $this->post_id = !empty($entity['post_id']) ? $entity['post_id'] : NULL;
    return $this->post_id;
  }

  /**
   * Save Apple News channels.
   */
  private function save_channels(array $channels = []) {
    $this->delete_channels();
    foreach ($channels as $channel) {
      $channel['post_id'] = $this->post_id;
      drupal_write_record('apple_news_channels', $channel);
    }
  }

  /**
   * Save Apple News sections.
   */
  private function save_sections(array $sections = []) {
    $this->delete_sections();
    foreach ($sections as $section) {
      $section['post_id'] = $this->post_id;
      drupal_write_record('apple_news_sections', $section);
    }
  }

  /**
   * Save Apple News metadata.
   */
  private function save_metadata(array $metadata = []) {
    $this->delete_metadata();
    $metadata['post_id'] = $this->post_id;
    drupal_write_record('apple_news_metadata', $metadata);
  }

  /**
   * Delete entities.
   */
  private function delete_entities() {
    db_delete('apple_news_entities')
      ->condition('post_id', $this->post_id)
      ->execute();
  }

  /**
   * Delete channels.
   */
  private function delete_channels() {
    db_delete('apple_news_channels')
      ->condition('post_id', $this->post_id)
      ->execute();
  }

  /**
   * Delete section information.
   */
  private function delete_sections() {
    db_delete('apple_news_sections')
      ->condition('post_id', $this->post_id)
      ->execute();
  }

  /**
   * Delete post metadata.
   */
  private function delete_metadata() {
    db_delete('apple_news_metadata')
      ->condition('post_id', $this->post_id)
      ->execute();
  }

  /**
   * Get article information.
   */
  public function getArticle() {
    $article = [];
    $query = db_select('apple_news_entities', 'e');
    $query->join('apple_news_channels', 'c', 'c.post_id = e.post_id');
    $query->join('apple_news_sections', 's', 's.post_id = e.post_id');
    $query->join('apple_news_metadata', 'm', 'm.post_id = e.post_id');
    $query->fields('e');
    $query->fields('c');
    $query->fields('s');
    $query->fields('m');
    $query->condition('e.post_id', $this->post_id);
    if (!empty($this->channel_id)) {
      if (is_array($this->channel_id)) {
        $query->condition('c.channel_id', $this->channel_id, 'IN');
      }
      else {
        $query->condition('c.channel_id', $this->channel_id);
      }
    }
    $query->orderBy('e.postdate', 'DESC');
    foreach ($query->execute() as $row) {
      $article = [
        'article_id'     => $row->article_id,
        'share_url'      => $row->share_url,
        'entity_id'      => $row->entity_id,
        'revision_id'    => $row->revision_id,
        'entity_type'    => $row->entity_type,
        'postdate'       => $row->postdate,
        'sections'       => [],
      ];
    }
    foreach ($query->execute() as $row) {
      $article['sections'][$row->section_id] = $row->channel_id;
    }
    return $article;
  }

  /**
   * Set entity id.
   */
  public function setEntityId($id) {
    $this->entity_id = $id;
  }

  /**
   * Set revision id.
   */
  public function setEntityRevisionId($id) {
    $this->revision_id = $id;
  }

  /**
   * Set entity type.
   */
  public function setEntityType($type) {
    $this->entity_type = $type;
  }

  /**
   * Set channel id.
   */
  public function setChannelId($id) {
    $this->channel_id = $id;
  }

  /**
   * Get primary key by entity information.
   */
  public function setPostId($entity_type, $entity_id, $revision_id) {
    $this->setEntityId($entity_id);
    $this->setEntityRevisionId($revision_id);
    $this->setEntityType($entity_type);

    $query = db_select('apple_news_entities', 'e');
    $query->join('apple_news_channels', 'c', 'c.post_id = e.post_id');
    $query->fields('e', ['post_id']);
    $query->fields('c');
    $query->condition('e.entity_type', $entity_type);
    $query->condition('e.entity_id', $entity_id);
    $query->condition('e.revision_id', $revision_id);
    if (!empty($this->channel_id)) {
      if (is_array($this->channel_id)) {
        $query->condition('c.channel_id', $this->channel_id, 'IN');
      }
      else {
        $query->condition('c.channel_id', $this->channel_id);
      }
    }

    $this->post_id = $query->execute()->fetchField();
    return $this->post_id;
  }

  /**
   * Get Apple News post by article ID.
   */
  public function setArticleId($article_id) {
    $this->post_id = db_select('apple_news_entities', 'e')
      ->fields('e', ['post_id'])
      ->condition('article_id', $article_id)
      ->execute()
      ->fetchField();
    return $this->getArticle();
  }

  /**
   * Save new Apple News post information.
   */
  public function insert($record) {
    $transaction = db_transaction();
    try {
      $this->save_entities($record['entity']);
      $this->save_channels($record['channels']);
      $this->save_sections($record['sections']);
      $this->save_metadata($record['metadata']);
    }
    catch (Exception $e) {
      $transaction->rollback();
      $msg = "Couldn't write post data for entity ID @id";
      $var = [
        '@id' => !empty($record['entity']['entity_id'])
                   ? $record['entity']['entity_id']
                   : 'N/A'
      ];
      watchdog_exception('apple_news', $e, $msg, $var, WATCHDOG_ERROR);
    }
  }

  /**
   * Update Apple News related tables.
   */
  public function update($record) {
    $transaction = db_transaction();
    try {
      $this->save_entities($record['entity'], 'post_id');
      $this->save_channels($record['channels']);
      $this->save_sections($record['sections']);
      $this->save_metadata($record['metadata']);
    }
    catch (Exception $e) {
      $transaction->rollback();
      $msg = "Couldn't write post ID @id";
      $var = [
        '@id' => $this->post_id
      ];
      watchdog_exception('apple_news', $e, $msg, $var, WATCHDOG_ERROR);
    }
  }

  /**
   * Delete Apple News related records.
   */
  public function delete() {
    $transaction = db_transaction();
    try {
      $this->delete_entities();
      $this->delete_channels();
      $this->delete_sections();
      $this->delete_metadata();
    }
    catch (Exception $e) {
      $transaction->rollback();
      $msg = "Couldn't delete post ID @id";
      $var = [
        '@id' => $this->post_id
      ];
      watchdog_exception('apple_news', $e, $msg, $var, WATCHDOG_ERROR);
    }
  }

}
