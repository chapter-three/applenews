<?php

/**
 * @file
 * Layout Destination classes.
 */

use \ChapterThree\AppleNews\Document\Layouts\Layout;
use \ChapterThree\AppleNews\Document\Layouts\ComponentLayout;

/**
 * Base class for layout destinations.
 *
 * All Layout destinations must extend this class.
 */
class ApplenewsDestinationLayout extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Configurable Layout') . ' ' . parent::label();
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'columns' => '7',
      'width'   => '1024',
      'margin'  => NULL,
      'gutter'  => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['columns'] = [
      '#title'         => t('Columns'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('columns'),
      '#required'      => TRUE,
    ];

    $form['width'] = [
      '#title'         => t('Width'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('width'),
      '#required'      => TRUE,
    ];

    $form['margin'] = [
      '#title'         => t('Margin'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('margin'),
    ];

    $form['gutter'] = [
      '#title'         => t('Gutter'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('gutter'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigForm($form, &$form_state) {
    // @todo use this pattern, after implementing exceptions in
    // \chapter-three\AppleNews\Document
    $layout = $this->export();
    if (!$layout instanceof Layout) {
      form_set_error(NULL, t('Could not create layout object.'));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    $layout = new Layout($this->getSetting('columns'), $this->getSetting('width'));
    foreach ([
        'margin' => 'setMargin',
        'gutter' => 'setGutter',
      ] as $key => $method
    ) {
      $value = $this->getSetting($key);
      if ($value !== NULL) {
        $layout->{$method}($value);
      }
    }
    return $layout;
  }

}

/**
 * Base class for component layout destinations.
 *
 * All ComponentLayout destinations must extend this class.
 */
class ApplenewsDestinationComponentLayout extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Configurable Component Layout') . ' ' . parent::label();
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'columnStart'                => NULL,
      'columnSpan'                 => NULL,
      'margin'                     => NULL,
      'contentInset'               => NULL,
      'ignoreDocumentMargin'       => NULL,
      'ignoreDocumentGutter'       => NULL,
      'minimumHeight'              => NULL,
      'maximumContentWidth'        => NULL,
      'horizontalContentAlignment' => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['id'] = [
      '#title'         => t('ID'),
      '#type'          => 'textfield',
      '#default_value' => $this->id,
      '#required'      => TRUE,
    ];

    $form['columnStart'] = [
      '#title'         => t('Column Start'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('columnStart'),
    ];

    $form['columnSpan'] = [
      '#title'         => t('Column Span'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('columnSpan'),
    ];

    $form['margin'] = [
      '#title'         => t('Margin'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('margin'),
    ];

    $form['contentInset'] = [
      '#title'         => t('Content Inset'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('contentInset'),
    ];

    $form['ignoreDocumentMargin'] = [
      '#title'         => t('Ignore Document Margin'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('ignoreDocumentMargin'),
    ];

    $form['ignoreDocumentGutter'] = [
      '#title'         => t('Ignore Document Gutter'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('ignoreDocumentGutter'),
    ];

    $form['minimumHeight'] = [
      '#title'         => t('Minimum Height'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('minimumHeight'),
    ];

    $form['maximumContentWidth'] = [
      '#title'         => t('Maximum Content Width'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('maximumContentWidth'),
    ];

    $form['horizontalContentAlignment'] = [
      '#title'         => t('Horizontal Content Alignment'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('horizontalContentAlignment'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigForm($form, &$form_state) {
    if ($form_state['values']['id'] != $this->id &&
        $this->export->getComponentLayout($form_state['values']['id'])
    ) {
      form_set_error('id', t('ID already in use.'));
    }
    // @todo use this pattern, after implementing exceptions in
    // \chapter-three\AppleNews\Document
    $layout = $this->export();
    if (!$layout instanceof Layout) {
      form_set_error(NULL, t('Could not create layout object.'));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigForm($form, &$form_state) {
    $this->export->deleteComponentLayout($this->id);
    $this->id = $form_state['values']['id'];
    $this->export->addComponentLayout($this, $this->id);
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    $layout = new ComponentLayout();
    foreach ([
        'columnStart'                => 'setColumnStart',
        'columnSpan'                 => 'setColumnSpan',
        'margin'                     => 'setMargin',
        'contentInset'               => 'setContentInset',
        'ignoreDocumentMargin'       => 'setIgnoreDocumentMargin',
        'ignoreDocumentGutter'       => 'setIgnoreDocumentGutter',
        'minimumHeight'              => 'setMinimumHeight',
        'maximumContentWidth'        => 'setMaximumContentWidth',
        'horizontalContentAlignment' => 'setHorizontalContentAlignment',
      ] as $key => $method
    ) {
      $value = $this->getSetting($key);
      if ($value !== NULL) {
        $layout->{$method}($value);
      }
    }
    return $layout;
  }

}