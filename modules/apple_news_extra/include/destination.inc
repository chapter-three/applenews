<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;

/**
 * Instagram component.
 */
class ApplenewsExtraDestinationInstagram extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'source' => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['source'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    parent::getConfigFormSourceElement($form, $form_state, 'source');
    $form['source']['#title'] = t('Link');
    $form['source']['#required'] = TRUE;
    $form['source']['#description'] = t('URL format <em>https://instagram.com/p/6Lg1xVkNjJ</em>');

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    list($key, $values) = $this->getSourceValues($this->export->source,
      $this->getSetting('source'));

    $components = [];

    foreach ($values as $value) {
      if (!empty($value[$key])) {
        $components[] = new Components\Instagram($value[$key]);
      }
    }

    return $components;
  }

}

/**
 * Tweet component.
 */
class ApplenewsExtraDestinationTweet extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'source' => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['source'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    parent::getConfigFormSourceElement($form, $form_state, 'source');
    $form['source']['#title'] = t('Link');
    $form['source']['#required'] = TRUE;
    $form['source']['#description'] = t('URL format <em>https://twitter.com/chapter_three/status/623547179563495424</em>');

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    list($key, $values) = $this->getSourceValues($this->export->source,
      $this->getSetting('source'));

    $components = [];

    foreach ($values as $value) {
      if (!empty($value[$key])) {
        $components[] = new Components\Tweet($value[$key]);
      }
    }

    return $components;
  }

}

/**
 * Embed Web Video component.
 */
class ApplenewsExtraDestinationEmbedWebVideo extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'video_url' => NULL,
      'caption'   => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['video_url'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    parent::getConfigFormSourceElement($form, $form_state, 'video_url');
    $form['video_url']['#title'] = t('Video URL');
    $form['video_url']['#required'] = TRUE;
    $form['video_url']['#description'] = t('URL format <em>https://www.youtube.com/embed/r4DQKtYL62Q</em> (Youtube) or <em>https://player.vimeo.com/video/121450839</em> (Vimeo)');

    parent::getConfigFormSourceElement($form, $form_state, 'caption');
    $form['caption']['#title'] = t('Caption');

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    list($key, $values) = $this->getSourceValues($this->export->source,
      $this->getSetting('video_url'));

    $components = [];

    foreach ($values as $value) {
      if (!empty($value[$key])) {
        $component = new Components\EmbedWebVideo($value[$key]);
        if (!empty($value['caption'])) {
          $component->setCaption($value['caption']);
        }
        $components[] = $component;
      }
    }

    return $components;
  }

}

/**
 * Audio component.
 */
class ApplenewsExtraDestinationAudio extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'type'            => 'music',
      'url'             => NULL,
      'caption'         => NULL,
      'imageURL'        => NULL,
      'explicitContent' => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['url'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title'         => t('Type'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => [
        'music' => t('Music'),
      ],
      '#default_value' => $this->getSetting('type'),
      '#description'   => t('The type of audio file.'),
    ];

    parent::getConfigFormSourceElement($form, $form_state, 'url');
    $form['url']['#title'] = t('Audio URL');
    $form['url']['#required'] = TRUE;

    parent::getConfigFormSourceElement($form, $form_state, 'imageURL');
    $form['imageURL']['#title'] = t('Image URL');

    parent::getConfigFormSourceElement($form, $form_state, 'caption');
    $form['caption']['#title'] = t('Caption');

    parent::getConfigFormSourceElement($form, $form_state, 'explicitContent');
    $form['explicitContent']['#title'] = t('Explicit Content');

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    list($key, $values) = $this->getSourceValues($this->export->source,
      $this->getSetting('url'));
    list($imageurl_key, $imageurl_values) = $this->getSourceValues(
      $this->export->source, $this->getSetting('imageURL'));
    list($caption_key, $caption_value) = $this->getSourceValues(
      $this->export->source, $this->getSetting('caption'));
    list($explicitcontent_key, $explicitcontent_value)
      = $this->getSourceValues($this->export->source,
          $this->getSetting('explicitContent'));

    $components = [];

    foreach ($values as $delta => $value) {
      if (!empty($value[$key])) {
        switch ($this->getSetting('type')) {

          case 'music':
            $url = $this->export->addAsset($value[$key]);
            $component = new Components\Music($url);
            if (!empty($caption_value[$delta][$caption_key])) {
              $component->setCaption($caption_value[$delta][$caption_key]);
            }
            if (!empty($imageurl_values[$delta][$imageurl_key])) {
              $url = $this->export->addAsset($imageurl_values[$delta][$imageurl_key]);
              $component->setImageURL($url);
            }
            if (!empty($explicitcontent_value[$delta][$explicitcontent_key])) {
              $component->setExplicitContent(TRUE);
            }
            $components[] = $component;
            break;

        }
      }
    }

    return $components;
  }

}
