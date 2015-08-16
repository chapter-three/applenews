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
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'source' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Instagram') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['source'] = [
      '#title' => t('Link'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['source'],
      '#description' => t('URL format <em>https://instagram.com/p/6Lg1xVkNjJ</em>'),
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
      $this->settings['source']);
    $components = [];
    foreach ($values as $value) {
      $components[] = new Components\Instagram($value[$key]);
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
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'source' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Tweet') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['source'] = [
      '#title' => t('Link'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['source'],
      '#description' => t('URL format <em>https://twitter.com/chapter_three/status/623547179563495424</em>'),
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
      $this->settings['source']);
    $components = [];
    foreach ($values as $value) {
      $components[] = new Components\Tweet($value[$key]);
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
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'video_url' => NULL,
      'caption' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Embed Web Video') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['video_url'] = [
      '#title' => t('Video URL'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['video_url'],
      '#description' => t('URL format <em>https://www.youtube.com/embed/r4DQKtYL62Q</em> (Youtube) or <em>https://player.vimeo.com/video/121450839</em> (Vimeo)'),
    ];

    $form['caption'] = [
      '#title' => t('Caption'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['caption'],
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
      $this->settings['video_url']);
    $components = [];
    foreach ($values as $value) {
      $component = new Components\EmbedWebVideo($value[$key]);
      if (!empty($value['caption'])) {
        $component->setCaption($value['caption']);
      }
      $components[] = $component;
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
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'type' => NULL,
      'url' => NULL,
      'caption' => NULL,
      'imageURL' => NULL,
      'explicitContent' => FALSE,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Audio') . ' (' . (!empty($this->settings['type']) ? $this->settings['type'] : $this->id) . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title' => t('Type'),
      '#type' => 'select',
      '#options' => [
        'Music' => t('Music'),
      ],
      '#default_value' => !empty($this->settings['type']) ? $this->settings['type'] : 'Music',
    ];

    $form['url'] = [
      '#title' => t('Audio URL'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['url'],
      '#description' => t('The URL of an audio file beginning with http:// or https://. This component supports all audio formats that are supported by AVPlayer on iOS devices. Supported audio file types include: MP3, AAC, ALAC, HE-AAC. See Supported Audio Formats developer documentation.'),
    ];

    $form['imageURL'] = [
      '#title' => t('Image URL'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['imageURL'],
      '#description' => t('An optional URL to an image that should accompany the audio file, such as a still or a cover image. Supported image types include JPEG (with .jpg or .jpeg extension), PNG, and GIF. <strong>Important:</strong> This field sould be a <em>File</em> field.'),
    ];

    $form['caption'] = [
      '#title' => t('Caption'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['caption'],
      '#description' => t('A caption that describes the content of the audio file. This text can be used by VoiceOver, or it can be shown when the audio cannot be played.'),
    ];

    $form['explicitContent'] = [
      '#title' => t('Explicit Content'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['explicitContent'],
      '#description' => t('This property indicates that the audio track may contain explicit/graphic content. Default value is False'),
    ];

    return $form;
  }

  /**
   * Exports to an Apple News Document container object.
   *
   * NOTE: This is a simple Audio field integration example.
   *       This example doesn't include cases with multiple audio files
   *       with multiple ExplicitContent and accessibilityCaption options
   *
   * @param mixed $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['url']);
    list($imageURL_key, $imageURL_values) = $this->getSourceValues($value,
      $this->settings['imageURL']);
    list($caption_key, $caption_value) = $this->getSourceValues($value,
      $this->settings['caption']);
    $components = [];
    foreach ($values as $value) {
      $file = $value['file'];
      if (is_object($file)) {
        switch ($this->settings['type']) {
          case 'Music' :
          default :
            $component = new Components\Music($value['url']);
            if (!empty($caption_value[0]['value'])) {
              $component->setCaption($caption_value[0]['value']);
            }
            if (!empty($imageURL_values[0]['file']) && is_object($imageURL_values[0]['file'])) {
              $component->setImageURL($this->export->addAsset($imageURL_values[0]['file']->uri));
            }
            if (!empty($this->settings['explicitContent'])) {
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
