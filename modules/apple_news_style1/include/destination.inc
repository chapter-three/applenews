<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Margin;
use \ChapterThree\AppleNews\Document\Components;
use \ChapterThree\AppleNews\Document\Styles;
use \ChapterThree\AppleNews\Document\Layouts;
use \ChapterThree\AppleNews\Document\Behaviors;
use \ChapterThree\AppleNews\Document\Animations;
use \ChapterThree\AppleNews\Document\Markdown;

/**
 * Body component.
 */
class Applenewsstyle1DestinationBody extends ApplenewsDestinationBodyPhoto {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Body') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {

    $body_text_style = new Styles\ComponentTextStyle();
    $body_text_style->setTextAlignment('left')
      ->setFontName('AvenirNext-Regular')
      ->setFontSize(16)
      ->setLineHeight(26)
      ->setTextColor('#000');

    $dc_style = new Styles\DropCapStyle(5);
    $dc_style->setNumberOfCharacters(1)
      ->setFontName('AvenirNext-UltraLight')
      ->setTextColor('#222E33')
      ->setPadding(5);
    $initial_body_text_style = clone($body_text_style);
    $initial_body_text_style->setDropCapStyle($dc_style);

    $body_inline_text_style = new Styles\InlineTextStyle(1, 19, $body_text_style);
    $body_animation = new Animations\ComponentAnimations\FadeInAnimation();
    $body_animation->setInitialAlpha(0.0)
      ->setUserControllable(TRUE);
    $body_layout = new Layouts\ComponentLayout();
    $body_layout->setMargin(new Margin(25, 25));

    $image_layout = new Layouts\ComponentLayout();

    switch ($this->export->getSetting('layout_alignment')) {

      case 'left':
        $body_layout->setColumnStart(0)->setColumnSpan(5);
        $image_layout->setColumnStart(3)->setColumnSpan(4);
        break;

      case 'center':
        $body_layout->setColumnStart(1)->setColumnSpan(7);
        $image_layout->setColumnStart(1)->setColumnSpan(7);
        break;

      case 'right':
        $body_layout->setColumnStart(2)->setColumnSpan(5);
        $image_layout->setColumnStart(0)->setColumnSpan(4);
        break;

    }

    $components = parent::export($value);
    for ($i = 0; $i < count($components); $i++) {
      $comp = $components[$i];
      if ($comp instanceof Components\Body) {
        /** @var \ChapterThree\AppleNews\Document\Components\Body $comp */
        if ($comp->getFormat() != 'markdown') {
          $comp->addInlineTextStyles($body_inline_text_style);
        }
        $comp->setLayout($body_layout)
          ->setAnimation($body_animation);
        if ($i) {
          $comp->setTextStyle($body_text_style);
        }
        else {
          $comp->setTextStyle($initial_body_text_style);
        }
      }
      elseif ($comp instanceof Components\ScalableImage) {
        /** @var \ChapterThree\AppleNews\Document\Components\ScalableImage $comp */
        $comp->setLayout($image_layout);
      }
    }

    return $components;
  }

}

/**
 * Image header component.
 */
class Applenewsstyle1DestinationHeaderImage extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'file' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Header Image') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['file'] = [
      '#title' => t('File'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['file'],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['file']);
    if (empty($values[0][$key])) {
      return [];
    }
    $file = $values[0][$key];
    if (is_object($file)) {
      $fill = new Styles\Fills\ImageFill($this->export->addAsset($file->uri));
    }
    else {
      $fill = new Styles\Fills\ImageFill($this->export->addAsset($file));
    }
    $fill->setFillMode('cover')->setVerticalAlignment('bottom');
    $style = new Styles\ComponentStyle();
    $style->setFill($fill);
    $header = new Components\Header();
    $header->setStyle($style);
    $layout = new Layouts\ComponentLayout();
    $layout->setIgnoreDocumentMargin()
      ->setMinimumHeight('42vh');
    $header->setLayout($layout);

    switch ($this->export->getSetting('layout_alignment')) {

      case 'left':
      case 'right':
        $layout->setColumnStart(0)->setColumnSpan(7);
        break;

      case 'center':
        $layout->setColumnStart(0)->setColumnSpan(9);
        break;

    }

    return [$header];
  }

}

/**
 * Gradient header container component.
 */
class Applenewsstyle1DestinationHeaderGradContainer extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'title' => NULL,
      'intro' => NULL,
      'author' => NULL,
      'photographer' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Header Gradient Container') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['title'] = [
      '#title' => t('Title'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['title'],
    ];

    $form['intro'] = [
      '#title' => t('Intro'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['intro'],
    ];

    $form['author'] = [
      '#title' => t('Author'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['author'],
    ];

    $form['photographer'] = [
      '#title' => t('Photographer'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['photographer'],
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
    // Components container.
    $container = new Components\Container();
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7);
    $container->setLayout($layout);
    $container->setBehavior(new Behaviors\BackgroundParallax());

    // Divider.
    $divider = new Components\Divider();
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7)
      ->setMargin(new Margin(NULL, 15));
    $divider->setLayout($layout);
    $stroke = new Styles\StrokeStyle();
    $stroke->setColor('#E6E6E6')->setWidth(1);
    $divider->setStroke($stroke);
    $container->addComponent($divider);

    // Title.
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['title']);
    if (empty($values[0][$key])) {
      return [];
    }
    $title = new Components\Title($values[0][$key]);
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7);
    $title->setLayout($layout);
    $text_style = new Styles\ComponentTextStyle();
    $text_style->setTextAlignment('center')
      ->setFontName('AvenirNext-UltraLight')
      ->setTextTransform('uppercase')
      ->setFontSize(64)
      ->setLineHeight(68)
      ->setTextColor('#FFF');
    $title->setTextStyle($text_style);
    $container->addComponent($title);

    // Divider.
    $divider = new Components\Divider();
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7)
      ->setMargin(new Margin(15, 15));
    $divider->setLayout($layout);
    $stroke = new Styles\StrokeStyle();
    $stroke->setColor('#E6E6E6')->setWidth(1);
    $divider->setStroke($stroke);
    $container->addComponent($divider);

    // Intro.
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['intro']);
    if (!empty($values[0][$key])) {
      $intro = new Components\Intro($values[0][$key]);
      $layout = new Layouts\ComponentLayout();
      $layout->setColumnStart(0)->setColumnSpan(7)
        ->setMargin(new Margin(10, 5));
      $intro->setLayout($layout);
      $text_style = new Styles\ComponentTextStyle();
      $text_style->setTextAlignment('center')
        ->setFontName('AvenirNext-Regular')
        ->setFontSize(18)
        ->setLineHeight(26)
        ->setTextColor('#FFF');
      $intro->setTextStyle($text_style);
      $container->addComponent($intro);
    }

    // Author.
    $text_style = new Styles\ComponentTextStyle();
    $text_style->setTextAlignment('center')
      ->setFontName('AvenirNextCondensed-DemiBold')
      ->setFontSize(13)
      ->setTextColor('#FFF');
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['author']);
    if (!empty($values[0][$key])) {
      $author = new Components\Author($values[0][$key]);
      $layout = new Layouts\ComponentLayout();
      $layout->setColumnStart(0)->setColumnSpan(7)
        ->setMargin(new Margin(10, 5));
      $author->setLayout($layout);
      $author->setTextStyle($text_style);
      $container->addComponent($author);
    }
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['photographer']);
    if (!empty($values[0][$key])) {
      $photog = new Components\Photographer($values[0][$key]);
      $layout = new Layouts\ComponentLayout();
      $layout->setColumnStart(0)->setColumnSpan(7)
        ->setMargin(new Margin(NULL, 15));
      $photog->setLayout($layout);
      $photog->setTextStyle($text_style);
      $container->addComponent($photog);
    }

    // Parent container.
    $header = new Components\Header();
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7)
      ->setIgnoreDocumentMargin()
      ->setMinimumHeight('80vh')
      ->setMargin(new Margin(NULL, 25));
    $header->setLayout($layout);
    $fill = new Styles\Fills\Gradients\LinearGradientFill([
      new Styles\Fills\Gradients\ColorStop('#263236'),
      new Styles\Fills\Gradients\ColorStop('#3B4A4E'),
      new Styles\Fills\Gradients\ColorStop('#6D8A95'),
      new Styles\Fills\Gradients\ColorStop('#FFFFFF'),
    ]);
    $fill->setAngle(180);
    $style = new Styles\ComponentStyle();
    $style->setFill($fill);
    $header->setStyle($style);
    $header->addComponent($container);
    return [$header];
  }

}

/**
 * Animated pullquote component.
 */
class Applenewsstyle1DestinationPullQuote extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'image' => NULL,
      'quote' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Pull Quote') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['image'] = [
      '#title' => t('Image'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['image'],
    ];

    $form['quote'] = [
      '#title' => t('Quote'),
      '#required' => TRUE,
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['quote'],
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

    list($key, $file) = $this->getSourceValues($value,
      $this->settings['image']);
    if (empty($file[0][$key])) {
      $file = NULL;
    }
    else {
      $file = $file[0][$key];
    }

    // Pullquote without background.
    if (empty($file)) {

      // Pullquote.
      list($key, $quote) = $this->getSourceValues($value,
        $this->settings['quote']);
      if (empty($quote[0][$key])) {
        return [];
      }
      $markdown = new Markdown();
      $quote = new Components\Pullquote($markdown->convert($quote[0][$key]));
      $quote->setFormat('markdown');
      $layout = new Layouts\ComponentLayout();
      $layout->setIgnoreDocumentMargin()
        ->setMinimumHeight('42vh');
      $quote->setLayout($layout);
      $text_style = new Styles\ComponentTextStyle();
      $text_style->setTextTransform('uppercase')
        ->setFontName('AvenirNext-Bold')
        ->setFontSize(48)
        ->setLineHeight(60);
      $quote->setTextStyle($text_style);

      switch ($this->export->getSetting('layout_alignment')) {

        case 'left':
          // Quote.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $quote->getLayout();
          $layout->setColumnStart(3)->setColumnSpan(4);
          /** @var \ChapterThree\AppleNews\Document\Styles\ComponentTextStyle $text_style */
          $text_style = $quote->getTextStyle();
          $text_style->setTextAlignment('left');
          break;

        case 'center':
          // Quote.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $quote->getLayout();
          $layout->setColumnStart(1)->setColumnSpan(7);
          /** @var \ChapterThree\AppleNews\Document\Styles\ComponentTextStyle $text_style */
          $text_style = $quote->getTextStyle();
          $text_style->setTextAlignment('center');
          break;

        case 'right':
          // Quote.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $quote->getLayout();
          $layout->setColumnStart(0)->setColumnSpan(4);
          /** @var \ChapterThree\AppleNews\Document\Styles\ComponentTextStyle $text_style */
          $text_style = $quote->getTextStyle();
          $text_style->setTextAlignment('left');
          break;

      }

      return [$quote];

    }
    // Pullquote with background.
    else {

      // Parent section.
      $section = new Components\Section();
      $section->setScene(new Animations\Scenes\ParallaxScaleHeader());
      $layout = new Layouts\ComponentLayout();
      $layout->setIgnoreDocumentMargin()
        ->setMinimumHeight('42vh');
      $section->setLayout($layout);

      // Header.
      if (is_object($file)) {
        $fill = new Styles\Fills\ImageFill($this->export->addAsset($file->uri));
      }
      else {
        $fill = new Styles\Fills\ImageFill($this->export->addAsset($file));
      }
      $fill->setFillMode('cover')->setVerticalAlignment('top');
      $style = new Styles\ComponentStyle();
      $style->setFill($fill);
      $header = new Components\Header();
      $header->setStyle($style);
      $layout = new Layouts\ComponentLayout();
      $layout->setIgnoreDocumentMargin()
        ->setMinimumHeight('75vw')
        ->setMargin(new Margin(25, 25));
      $header->setLayout($layout);

      // Pullquote.
      list($key, $quote) = $this->getSourceValues($value,
        $this->settings['quote']);
      if (empty($quote[0][$key])) {
        return [];
      }
      $markdown = new Markdown();
      $quote = new Components\Pullquote($markdown->convert($quote[0][$key]));
      $quote->setFormat('markdown');
      $layout = new Layouts\ComponentLayout();
      $layout->setColumnStart(0)->setColumnSpan(7)->setMargin(new Margin(70));
      $quote->setLayout($layout);
      $quote->setBehavior(new Behaviors\BackgroundParallax());
      $text_style = new Styles\ComponentTextStyle();
      $text_style->setTextTransform('uppercase')
        ->setFontName('AvenirNext-Bold')
        ->setFontSize(16)
        ->setLineHeight(20)
        ->setTextColor('#FFF');
      $quote->setTextStyle($text_style);

      $header->addComponent($quote);
      $section->addComponent($header);

      switch ($this->export->getSetting('layout_alignment')) {

        case 'left':
          // Quote.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $quote->getLayout();
          $layout->setColumnStart(1);
          /** @var \ChapterThree\AppleNews\Document\Styles\ComponentTextStyle $text_style */
          $text_style = $quote->getTextStyle();
          $text_style->setTextAlignment('left');
          // Section.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $section->getLayout();
          $layout->setColumnStart(3)->setColumnSpan(4);
          break;

        case 'center':
          // Quote.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $quote->getLayout();
          $layout->setColumnStart(1)->setColumnSpan(5);
          /** @var \ChapterThree\AppleNews\Document\Styles\ComponentTextStyle $text_style */
          $text_style = $quote->getTextStyle();
          $text_style->setTextAlignment('center');
          // Section.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $section->getLayout();
          $layout->setColumnStart(1)->setColumnSpan(7);
          break;

        case 'right':
          // Quote.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $quote->getLayout();
          $layout->setColumnSpan(6);
          /** @var \ChapterThree\AppleNews\Document\Styles\ComponentTextStyle $text_style */
          $text_style = $quote->getTextStyle();
          $text_style->setTextAlignment('left');
          // Section.
          /** @var \ChapterThree\AppleNews\Document\Layouts\ComponentLayout $layout */
          $layout = $section->getLayout();
          $layout->setColumnStart(0)->setColumnSpan(4);
          break;

      }

      return [$section];

    }

  }

}

/**
 * Mosaic or Gallery component.
 */
class Applenewsstyle1DestinationImages extends ApplenewsDestinationImages {

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
    $components = parent::export($value);
    if (empty($components)) {
      return [];
    }
    /** @var \ChapterThree\AppleNews\Document\Components\Mosaic|\ChapterThree\AppleNews\Document\Components\Gallery $images */
    list($images) = $components;
    $animation = new Animations\ComponentAnimations\ScaleFadeAnimation();
    $animation->setInitialAlpha(0.5)->setInitialScale(0.75);
    $images->setAnimation($animation);
    $layout = new Layouts\ComponentLayout();
    $layout
      ->setIgnoreDocumentMargin()
      ->setMargin(new Margin(25, 25));
    $images->setLayout($layout);

    switch ($this->export->getSetting('layout_alignment')) {

      case 'left':
      case 'right':
        $layout->setColumnStart(0)->setColumnSpan(7);
        break;

      case 'center':
        $layout->setColumnStart(0)->setColumnSpan(9);
        break;

    }

    return [$images];
  }

}

/**
 * Banner Ad.
 */
class Applenewsstyle1DestinationAd extends ApplenewsDestinationAd {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings['type'] = 'banner';
    $this->settings['banner_type'] = 'standard';
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Banner Ad') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {
    $form = parent::getConfigForm($form, $form_state);
    $form['type']['#type'] = 'value';
    $form['banner_type']['#type'] = 'value';
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    $ad = parent::export($value);
    if (empty($ad)) {
      return [];
    }
    /** @var \ChapterThree\AppleNews\Document\Components\Advertisements\BannerAdvertisement $ad */
    $ad = $ad[0];

    $layout = new Layouts\ComponentLayout();
    $ad->setLayout($layout);

    switch ($this->export->getSetting('layout_alignment')) {

      case 'left':
      case 'right':
        $layout->setColumnStart(0)->setColumnSpan(7);
        break;

      case 'center':
        $layout->setColumnStart(1)->setColumnSpan(7);
        break;

    }

    return [$ad];
  }

}
