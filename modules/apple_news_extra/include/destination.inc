<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;

/**
 * Instagram component.
 */
class ApplenewsExtraDestinationInstagram extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Instagram') . ' (' . $this->id . ')';
  }

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
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['source'] = [
      '#title'         => t('Link'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('source'),
      '#description'   => t('URL format <em>https://instagram.com/p/6Lg1xVkNjJ</em>'),
    ];

    return $form;
  }

  /**
   * Exports to an Apple News Document container object.
   *
   * @param mixed $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {

    list($key, $values) = $this->getSourceValues($value,
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
class ApplenewsExtraDestinationTweet extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Tweet') . ' (' . $this->id . ')';
  }

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
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['source'] = [
      '#title'         => t('Link'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('source'),
      '#description'   => t('URL format <em>https://twitter.com/chapter_three/status/623547179563495424</em>'),
    ];

    return $form;
  }

  /**
   * Exports to an Apple News Document container object.
   *
   * @param mixed $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {

    list($key, $values) = $this->getSourceValues($value,
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
class ApplenewsExtraDestinationEmbedWebVideo extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Embed Web Video') . ' (' . $this->id . ')';
  }

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
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['video_url'] = [
      '#title'         => t('Video URL'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('video_url'),
      '#description'   => t('URL format <em>https://www.youtube.com/embed/r4DQKtYL62Q</em> (Youtube) or <em>https://player.vimeo.com/video/121450839</em> (Vimeo)'),
    ];

    $form['caption'] = [
      '#title'         => t('Caption'),
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('caption'),
      '#description'   => t('A caption that describes the content of the video. This text can be used by VoiceOver, or it can be shown when the video cannot be played.'),
    ];

    return $form;
  }

  /**
   * Exports to an Apple News Document container object.
   *
   * @param mixed $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
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
class ApplenewsExtraDestinationAudio extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Audio') . ' ' . $this->getSetting('type') .
      ' (' . $this->id . ')';
  }

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

    $form['url'] = [
      '#title'         => t('Audio URL'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('url'),
      '#description'   => t('The URL of an audio file beginning with http:// or https://. This component supports all audio formats that are supported by AVPlayer on iOS devices. Supported audio file types include: MP3, AAC, ALAC, HE-AAC (For a full list of supported codecs, refer to the <a href="!link" target="_blank">Supported Audio Formats developer documentation</a>).',
        [
          '!link' => 'https://developer.apple.com/library/ios/documentation/AudioVideo/Conceptual/MultimediaPG/UsingAudio/UsingAudio.html#//apple_ref/doc/uid/TP40009767-CH2-SW48',
        ]
      ),
    ];

    $form['imageURL'] = [
      '#title'         => t('Image URL'),
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('imageURL'),
      '#description'   => t('Use File field. An optional URL to an image that should accompany the audio file, such as a still or a cover image. Supported image types include JPEG (with .jpg or .jpeg extension), PNG, and GIF.'),
    ];

    $form['caption'] = [
      '#title'         => t('Caption'),
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('caption'),
      '#description'   => t('A caption that describes the content of the audio file. This text can be used by VoiceOver, or it can be shown when the audio cannot be played.'),
    ];

    $form['explicitContent'] = [
      '#title'         => t('Explicit Content'),
      '#type'          => 'select',
      '#options'       => $this->getSourceOptions(),
      '#default_value' => $this->getSetting('explicitContent'),
      '#description'   => t('This property indicates that the audio track may contain explicit/graphic content. Default value: false.'),
    ];

    return $form;
  }

  /**
   * Exports to an Apple News Document container object.
   *
   * @param mixed $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->getSetting('url'));
    list($imageurl_key, $imageurl_values) = $this->getSourceValues($value,
      $this->getSetting('imageURL'));
    list($caption_key, $caption_value) = $this->getSourceValues($value,
      $this->getSetting('caption'));
    list($explicitcontent_key, $explicitcontent_value)
      = $this->getSourceValues($value,
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
            if (!empty($imageurl_values[$delta][$imageurl_key]) &&
                is_object($imageurl_values[$delta][$imageurl_key])
            ) {
              $component->setImageURL($this->export->addAsset($imageurl_values[$delta]['file']->uri));
            }
            if (!empty($explicitcontent_value[$delta][$explicitcontent_key])) {
              $component->setExplicitContent(TRUE);
            }
            break;

        }
        $components[] = $component;
      }
    }
    return $components;
  }

}
