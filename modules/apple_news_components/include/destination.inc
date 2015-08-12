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
    $source = $this->getSource($value, $this->settings['file']);
    $key = $this->getSourceKey($this->settings['file']);
    $file = $source->export()[$key];
    $fill = new Styles\Fills\ImageFill($this->export->addAsset($file->uri));
    $fill->setFillMode('cover')->setVerticalAlignment('bottom');
    $style = new Styles\ComponentStyle();
    $style->setFill($fill);
    $header = new Components\Header();
    $header->setStyle($style);
    //  ->setLayout('headerImageContainerLayout');
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
   * @param array $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {
    // Components container
    $container = new Components\Header();
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
    $source = $this->getSource($value, $this->settings['title']);
    $key = $this->getSourceKey($this->settings['title']);
    $title = $source->export()[$key];
    $title = new Components\Title($title);
    $container->addComponent($title);

    // Another, slightly different, divider.
    $divider = clone $divider;
    $divider->getLayout()
      ->setMargin(new Margin(15, 15));
    $container->addComponent($divider);

    // Intro.
    $source = $this->getSource($value, $this->settings['intro']);
    $key = $this->getSourceKey($this->settings['intro']);
    $intro = $source->export()[$key];
    if ($intro) {
      $intro = new Components\Intro($intro);
      // $intro->setLayout('introLayout')->setTextStyle('introStyle');
      $container->addComponent($intro);
    }

    // Author.
    $source = $this->getSource($value, $this->settings['author']);
    $key = $this->getSourceKey($this->settings['author']);
    $author = $source->export()[$key];
    $author = new Components\Author($author);
    $layout = new Layouts\ComponentLayout();
    $layout->setColumnStart(0)->setColumnSpan(7)
      ->setMargin(new Margin(10, 5));
    $author->setLayout($layout);
    // $author->setTextStyle('authorStyle');
    $container->addComponent($author);

    // Photographer.
    $source = $this->getSource($value, $this->settings['photographer']);
    $key = $this->getSourceKey($this->settings['photographer']);
    $photog = $source->export()[$key];
    if ($photog) {
      $photog = new Components\Photographer($photog);
      $layout = new Layouts\ComponentLayout();
      $layout->setColumnStart(0)->setColumnSpan(7)
        ->setMargin(new Margin(NULL, 15));
      $photog->setLayout($layout);
      // $photog->setTextStyle('authorStyle');
      $container->addComponent($photog);
    }

    // Parent container.
    $header = new Components\Container();
    // $header->setLayout('headerGradientContainerLayout')
    //   ->setStyle('headerGradientContainerStyle');
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
   * @param array $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {
    // Pullquote.
    $source = $this->getSource($value, $this->settings['quote']);
    $key = $this->getSourceKey($this->settings['quote']);
    $quote = $source->export()[$key];
    if (empty($quote)) {
      return [];
    }

    $quote = new Components\Pullquote($quote);
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
    $source = $this->getSource($value, $this->settings['image']);
    $key = $this->getSourceKey($this->settings['image']);
    $file = $source->export()[$key];
    $fill = new Styles\Fills\ImageFill($this->export->addAsset($file->uri));
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
