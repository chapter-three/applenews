<?php

/**
 * @file
 * PublisherAPI class integration.
 */

/**
 * Gets a news api object.
 *
 * @return \ChapterThree\AppleNewsAPI\PublisherAPI
 *   An initialized api object.
 */
function applenews_publisher_api() {

  $key = variable_get('applenews_variable_api_key', FALSE);
  $secret = variable_get('applenews_variable_api_secret', FALSE);
  $endpoint = variable_get('applenews_variable_api_endpoint', FALSE);

  $PublisherAPI = new ChapterThree\AppleNewsAPI\PublisherAPI($key, $secret, $endpoint);

  // Allow configuring curl for debugging.
  $opts = variable_get('applenews_variable_api_curlopt',
    [
      'ssl'        => TRUE,
      'proxy'      => '',
      'proxy_port' => '',
    ]
  );

  /** @var \Curl\Curl $client */
  $client = $PublisherAPI->client;
  $client->setOpt(CURLOPT_SSL_VERIFYPEER, $opts['ssl']);
  if (!empty($opts['proxy'])) {
    $client->setOpt(CURLOPT_PROXY, $opts['proxy']);
  }
  if (!empty($opts['proxy_port'])) {
    $client->setOpt(CURLOPT_PROXYPORT, $opts['proxy_port']);
  }

  return $PublisherAPI;
}

/**
 * Fetches information about a channel.
 */
function applenews_get_channel($channel_id) {
  $PublisherAPI = applenews_publisher_api();
  $response = $PublisherAPI->get('/channels/{channel_id}',
    [
      'channel_id' => $channel_id
    ]
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('applenews', $PublisherAPI->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches a list of all sections for a channel.
 */
function applenews_get_sections($channel_id) {
  $PublisherAPI = applenews_publisher_api();
  $response = $PublisherAPI->get('/channels/{channel_id}/sections',
    [
      'channel_id' => $channel_id
    ]
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('applenews', $PublisherAPI->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches information about a single section.
 */
function applenews_get_section($section_id) {
  $PublisherAPI = applenews_publisher_api();
  $response = $PublisherAPI->get('/sections/{section_id}',
    [
      'section_id' => $section_id
    ]
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('applenews', $PublisherAPI->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Fetches an article.
 *
 * @return mixed
 *   Response object, FALSE if not found, or NULL if error.
 */
function applenews_get_article($article_id) {
  $PublisherAPI = applenews_publisher_api();
  $response = $PublisherAPI->get('/articles/{article_id}',
    [
      'article_id' => $article_id
    ]
  );
  if (empty($response)) {
    /** @var \Curl\Curl $client */
    watchdog('applenews', $PublisherAPI->client->errorMessage, [], WATCHDOG_ERROR);
    return NULL;
  }
  elseif (!empty($response->errors)) {
    foreach ($response->errors as $err) {
      switch ($err->code) {
        case 'NOT_FOUND':
          return FALSE;
        default:
          watchdog('applenews', $err->code, [], WATCHDOG_ERROR);
          return NULL;
      }
    }
  }
  return $response;
}

/**
 * Update existing article.
 */
function applenews_update_article($article_id, $data = []) {
  $PublisherAPI = applenews_publisher_api();
  $response = $PublisherAPI->post('/articles/{article_id}',
    [
      'article_id' => $article_id
    ],
    $data
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('applenews', $PublisherAPI->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Publishes a new article to a channel.
 */
function applenews_post_article($channel_id, $data = []) {
  $PublisherAPI = applenews_publisher_api();
  $response = $PublisherAPI->post('/channels/{channel_id}/articles',
    [
      'channel_id' => $channel_id
    ],
    $data
  );
  if (!$response) {
    /** @var \Curl\Curl $client */
    watchdog('applenews', $PublisherAPI->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}

/**
 * Deletes an article.
 */
function applenews_delete_article($article_id) {
  $PublisherAPI = applenews_publisher_api();
  $response = $PublisherAPI->delete('/articles/{article_id}',
    [
      'article_id' => $article_id
    ]
  );
  if (!empty($response)) {
    /** @var \Curl\Curl $client */
    watchdog('applenews', $PublisherAPI->client->errorMessage, [], WATCHDOG_ERROR);
    return FALSE;
  }
  return $response;
}
