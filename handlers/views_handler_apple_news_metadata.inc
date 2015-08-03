<?php

/**
 * @file
 * Definition of views_handler_apple_news_metadata.
 */

/**
 * Field handler to display Apple News metadata.
 *
 * @ingroup views_field_handlers
 */
class views_handler_apple_news_metadata extends views_handler_field {

  function query() {
    // do nothing -- to override the parent query.
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['metadata'] = array('default' => 'shareUrl');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    module_load_include('inc', 'apple_news');
    $form['metadata'] = [
      '#type' => 'select',
      '#title' => t('Apple News Metadata'),
      '#options' => [
        'shareUrl' => t('Share URL link'),
        'channel'  => t('Feeds'),
        'sections' => t('Sections'),
      ],
      '#default_value' => $this->options['metadata'],
    ];
  }

  function render($record) {
    module_load_include('inc', 'apple_news');
    $output = '';
    $article_id = $record->apple_news_published_entities_article_id;
    $article_revision_id = $record->apple_news_published_entities_article_revision_id;
    // Load apple news metadata.
    $metadata = apple_news_article_metadata($article_id, $article_revision_id);
    // Dislay metadata.
    switch ($this->options['metadata']) {
      case 'shareUrl' :
        if (!empty($metadata['shareUrl'])) {
          $output = l($metadata['shareUrl'], $metadata['shareUrl'],
            [
              'absolute' => TRUE,
              'attributes' => [
                'target' => '_blank',
              ]
            ]
          );
        }
        break;
      default :
        $output = filter_xss($metadata[$this->options['metadata']]);
        break;
    }
    return $output;
  }

}
