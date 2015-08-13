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

  $pushapi = new ChapterThree\AppleNews\PushAPI($key, $secret, $endpoint);

  // Allow configuring curl for debugging.
  $opts = variable_get('apple_news_variable_api_curlopt', ['ssl' => FALSE]);

  /** @var \Curl\Curl $client */
  $client = $pushapi->client;
  foreach ($opts as $key => $val) {
    $client->setOpt($key, $val);
  }

  return $pushapi;
}

/**
 * Fetches information about a channel.
 */
function apple_news_get_channel($channel_id) {
  $pushapi = apple_news_push_api();
  $response = $pushapi->get('/channels/{channel_id}',
    [
      'channel_id' => $channel_id
    ]
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $pushapi->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches a list of all sections for a channel.
 */
function apple_news_get_sections($channel_id) {
  $pushapi = apple_news_push_api();
  $response = $pushapi->get('/channels/{channel_id}/sections',
    [
      'channel_id' => $channel_id
    ]
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $pushapi->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches information about a single section.
 */
function apple_news_get_section($section_id) {
  $pushapi = apple_news_push_api();
  $response = $pushapi->get('/sections/{section_id}',
    [
      'section_id' => $section_id
    ]
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $pushapi->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches an article.
 */
function apple_news_get_article($article_id) {
  $pushapi = apple_news_push_api();
  $response = $pushapi->get('/articles/{article_id}',
    [
      'article_id' => $article_id
    ]
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $pushapi->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Update existing article.
 */
function apple_news_update_article($article_id, $data = []) {
  $pushapi = apple_news_push_api();
  $response = $pushapi->post('/articles/{article_id}',
    [
      'article_id' => $article_id
    ],
    $data
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $pushapi->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Publishes a new article to a channel.
 */
function apple_news_post_article($channel_id, $data = []) {
  $pushapi = apple_news_push_api();
  $response = $pushapi->post('/channels/{channel_id}/articles',
    [
      'channel_id' => $channel_id
    ],
    $data
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $pushapi->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Deletes an article.
 */
function apple_news_delete_article($article_id) {
  $pushapi = apple_news_push_api();
  $response = $pushapi->delete('/articles/{article_id}',
    [
      'article_id' => $article_id
    ]
  );
  if (!empty($response)) {
    /** @var \Curl\Curl $client */
    watchdog('apple_news', $pushapi->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}
