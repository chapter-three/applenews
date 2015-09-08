<?php

/**
 * @file
 * Apple News DB class.
 *
 * This is a helper class to populate various Apple News
 * related database tables.
 */

interface ApplenewsDatabaseInterface {

  /**
   * Get article object.
   */
  public function getArticle();

  /**
   * Set entity ID.
   */
  public function setEntityId($id);

  /**
   * Set entity revision ID.
   */
  public function setEntityRevisionId($id);

  /**
   * Set entity type.
   */
  public function setEntityType($type);

  /**
   * Set channel ID.
   */
  public function setChannelId($id);

  /**
   * Get entity channels.
   */
  public function getChannels();

  /**
   * Set post ID (primary key value).
   * Returns post ID by entity type, entity ID, entity revision ID.
   */
  public function setPostId($entity_type, $entity_id, $revision_id);

  /**
   * Get article by article ID.
   */
  public function getArticleById($article_id);

  /**
   * Save new Apple News post information.
   */
  public function insert($record);

  /**
   * Update Apple News related tables.
   */
  public function update($record);

  /**
   * Delete Apple News related records.
   */
  public function delete();

  /**
   * Delete Apple News related records by Entity ID and Entity Type.
   */
  public function deleteByEntity($entity_id, $entity_type);

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
  private $exid = NULL;

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
    $entity['exid'] = $this->exid;
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
   * Save export for an entity.
   */
  public function setExport($exid) {
    $this->exid = $exid;
    $entity = [
      'post_id' => $this->post_id,
      'exid' => $exid,
    ];
    drupal_write_record('apple_news_entities', $entity, 'post_id');
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
        'exid'           => $row->exid,
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
   * Get channels.
   */
  public function getChannels() {
    $channel_ids = [];

    $query = db_select('apple_news_entities', 'e');
    $query->join('apple_news_channels', 'c', 'c.post_id = e.post_id');
    $query->fields('c', ['channel_id']);
    $query->condition('e.entity_type', $this->entity_type);
    $query->condition('e.entity_id', $this->entity_id);
    $query->condition('e.revision_id', $this->revision_id);

    if (!empty($this->channel_id)) {
      if (is_array($this->channel_id)) {
        $query->condition('c.channel_id', $this->channel_id, 'IN');
      }
      else {
        $query->condition('c.channel_id', $this->channel_id);
      }
    }
    $result = $query->execute();
    foreach ($result as $row) {
      $channel_ids[] = $row->channel_id;
    }
    return $channel_ids;
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
    $query->fields('e', ['post_id', 'exid']);
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
    if ($result = $query->execute()->fetchAssoc()) {
      $this->post_id = $result['post_id'];
      $this->exid = $result['exid'];
    }
    return $this->post_id;
  }

  /**
   * Get article by article ID.
   */
  public function getArticleById($article_id) {
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

  /**
   * Delete Apple News related records by Entity ID and Entity Type.
   */
  public function deleteByEntity($entity_wrapper, $entity_type) {
    $transaction = db_transaction();
    try {
      $entity_id = $entity_wrapper->getIdentifier();

      $result = db_select('apple_news_entities', 'e')
        ->fields('e', ['post_id'])
        ->condition('e.entity_id', $entity_id)
        ->condition('e.entity_type', $entity_type)
        ->execute();
      foreach ($result as $row) {
        $this->post_id = $row->post_id;
        $this->delete_entities();
        $this->delete_channels();
        $this->delete_sections();
        $this->delete_metadata();
      }
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
