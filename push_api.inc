<?php

/**
 * @file
 * PushAPI class integration.
 */

/**
 * Gets a news api object.
 *
 * @return \ChapterThree\AppleNews\PushAPI
 *   An initialized api object.
 */
function apple_news_push_api() {

  $key = variable_get('apple_news_variable_api_key', FALSE);
  $secret = variable_get('apple_news_variable_api_secret', FALSE);
  $endpoint = variable_get('apple_news_variable_api_endpoint', FALSE);

  $PushAPI = new ChapterThree\AppleNews\PushAPI($key, $secret, $endpoint);

  // Allow configuring curl for debugging.
  $opts = variable_get('apple_news_variable_api_curlopt', array());

  /** @var \Curl\Curl $client */
  $client = $PushAPI->client;
  foreach ($opts as $key => $val) {
    $client->setOpt($key, $val);
  }

  return $PushAPI;
}

/**
 * Fetches information about a channel.
 */
function apple_news_get_channel($channel_id) {
  $PushAPI = apple_news_push_api();
  $response = $PushAPI->get('/channels/{channel_id}', array('channel_id' => $channel_id));
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $PushAPI->client->curl_error_message, array(), WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches a list of all sections for a channel.
 */
function apple_news_get_sections($channel_id) {
  $PushAPI = apple_news_push_api();
  $response = $PushAPI->get('/channels/{channel_id}/sections', array('channel_id' => $channel_id));
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $PushAPI->client->curl_error_message, array(), WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches information about a single section.
 */
function apple_news_get_section($section_id) {
  $PushAPI = apple_news_push_api();
  $response = $PushAPI->get('/sections/{section_id}', array('section_id' => $section_id));
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $PushAPI->client->curl_error_message, array(), WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches an article.
 */
function apple_news_get_article($article_id) {
  $PushAPI = apple_news_push_api();
  $response = $PushAPI->get('/articles/{article_id}', array('article_id' => $article_id));
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $PushAPI->client->curl_error_message, array(), WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Publishes a new article to a channel.
 */
function apple_news_post_article($channel_id, $data = array()) {
  $PushAPI = apple_news_push_api();
  $response = $PushAPI->post('/channels/{channel_id}/articles',
      array(
        'channel_id' => $channel_id
      ),
    $data
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $PushAPI->client->curl_error_message, array(), WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Deletes an article.
 */
function apple_news_delete_article($article_id) {
  $PushAPI = apple_news_push_api();
  $response = $PushAPI->delete('/articles/{article_id}', array('article_id' => $article_id));
  if (!empty($response)) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $PushAPI->client->curl_error_message, array(), WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}
