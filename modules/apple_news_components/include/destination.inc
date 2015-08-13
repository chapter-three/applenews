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

/**
 * Body component.
 */
class ApplenewscomponentsDestinationBody extends ApplenewsDestinationBodyPhoto {

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
    $text_style = new Styles\TextStyle();
    $text_style->setTextTransform('uppercase')
      ->setTextColor('#FF00007F');
    $inline_text_style = new Styles\InlineTextStyle(1, 19, $text_style);
    $animation = new Animations\ComponentAnimations\FadeInAnimation();
    $animation->setInitialAlpha(0.0)
      ->setUserControllable(TRUE);

    $components = parent::export($value);
    foreach ($components as $component) {
      if (is_a($component, '\ChapterThree\AppleNews\Document\Components\Body')) {
        /** @var \ChapterThree\AppleNews\Document\Components\Body $component */
        $component
          ->addInlineTextStyles($inline_text_style)
          ->setTextStyle($text_style)
          ->setAnimation($animation);
      }
    }

    return $components;
  }

}

/**
 * Image header component.
 */
class ApplenewscomponentsDestinationHeaderImage extends ApplenewsDestination {

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
    // ->setLayout('headerImageContainerLayout');
    return [$header];
  }

}

/**
 * Gradient header container component.
 */
class ApplenewscomponentsDestinationHeaderGradContainer extends ApplenewsDestination {

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
    // $container->setLayout('titleContainerLayout');
    $behavior = new Behaviors\Parallax();
    $behavior->setFactor(0.7);
    $container->setBehavior($behavior);

    // Divider.
    $divider = new Components\Divider();
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7)
      ->setMargin(new Margin(NULL, 15));
    $divider->setLayout($layout);
    $stroke = new Styles\StrokeStyle();
    $stroke->setColor('#FFF')->setWidth(1);
    $divider->setStroke($stroke);
    $container->addComponent($divider);

    // Title.
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['title']);
    if (empty($values[0][$key])) {
      return [];
    }
    $title = new Components\Title($values[0][$key]);
    $container->addComponent($title);

    // Divider.
    $divider = new Components\Divider();
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7)
      ->setMargin(new Margin(15, 15));
    $divider->setLayout($layout);
    $stroke = new Styles\StrokeStyle();
    $stroke->setColor('#FFF')->setWidth(1);
    $divider->setStroke($stroke);
    $container->addComponent($divider);

    // Intro.
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['intro']);
    if (!empty($values[0][$key])) {
      $intro = new Components\Intro($values[0][$key]);
      // $intro->setLayout('introLayout')->setTextStyle('introStyle');
      $container->addComponent($intro);
    }

    // Author.
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['author']);
    if (!empty($values[0][$key])) {
      $author = new Components\Author($values[0][$key]);
      $layout = new Layouts\ComponentLayout();
      $layout->setColumnStart(0)->setColumnSpan(7)
        ->setMargin(new Margin(10, 5));
      $author->setLayout($layout);
      // $author->setTextStyle('authorStyle');
      $container->addComponent($author);
    }

    // Photographer.
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['photographer']);
    if (!empty($values[0][$key])) {
      $photog = new Components\Photographer($values[0][$key]);
      $layout = new Layouts\ComponentLayout();
      $layout->setColumnStart(0)->setColumnSpan(7)
        ->setMargin(new Margin(NULL, 15));
      $photog->setLayout($layout);
      // $photog->setTextStyle('authorStyle');
      $container->addComponent($photog);
    }

    // Parent container.
    $header = new Components\Header();
    // $header->setLayout('headerGradientContainerLayout')
    // ->setStyle('headerGradientContainerStyle');
    $header->addComponent($container);
    return [$header];
  }

}

/**
 * Animated pullquote component.
 */
class ApplenewscomponentsDestinationAnimatedPullQuote extends ApplenewsDestination {

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
    return t('Animated Pull Quote') . ' (' . $this->id . ')';
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
    // Pullquote.
    list($key, $quote) = $this->getSourceValues($value,
      $this->settings['quote']);
    if (empty($quote[0][$key])) {
      return [];
    }
    $quote = new Components\Pullquote($quote[0][$key]);
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7)->setMargin(new Margin(70));
    $quote->setLayout($layout);
    $behavior = new Behaviors\Parallax();
    $behavior->setFactor(1.1);
    $quote->setBehavior($behavior);
    // $quote->setStyle('pullquoteWhiteStyle');

    // Parent section.
    $section = new Components\Section('animatedPullQuote');
    // $section->setLayout('headerImageContainerLayout');
    $section->setScene(new Animations\Scenes\ParallaxScaleHeader());

    // Header.
    list($key, $file) = $this->getSourceValues($value,
      $this->settings['image']);
    if (empty($file[0][$key])) {
      return [];
    }
    $file = $file[0][$key];
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

    $header->addComponent($quote);
    $section->addComponent($header);

    return [$section];
  }

}

/**
 * Mosaic component.
 */
class ApplenewscomponentsDestinationMosaic extends ApplenewsDestinationMosaic {

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
    /** @var \ChapterThree\AppleNews\Document\Components\Mosaic $mosaic */
    list($mosaic) = $components;
    // $mosaic->setLayout('galleryLayout');
    $animation = new Animations\ComponentAnimations\ScaleFadeAnimation();
    $animation->setInitialAlpha(0.5)->setInitialScale(0.75);
    $mosaic->setAnimation($animation);
    return [$mosaic];
  }

}
