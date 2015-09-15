<?php

/**
 * @file
 * Layout Destination classes.
 */

use \ChapterThree\AppleNewsAPI\Document\Layouts\Layout;
use \ChapterThree\AppleNewsAPI\Document\Layouts\ComponentLayout;

/**
 * Base class for layout destinations.
 *
 * All Layout destinations must extend this class.
 */
class ApplenewsDestinationLayout extends ApplenewsDestinationSingleton {

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
  public function getConfigForm(array $values = []) {
    $form = parent::getConfigForm($values);

    $form['settings']['columns'] = [
      '#title'         => t('Columns'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('columns'),
      '#required'      => TRUE,
    ];

    $form['settings']['width'] = [
      '#title'         => t('Width'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('width'),
      '#required'      => TRUE,
    ];

    $form['settings']['margin'] = [
      '#title'         => t('Margin'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('margin'),
    ];

    $form['settings']['gutter'] = [
      '#title'         => t('Gutter'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('gutter'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    $layout = new Layout($this->getSetting('columns'), $this->getSetting('width'));
    foreach ([
        'margin' => 'setMargin',
        'gutter' => 'setGutter',
      ] as $key => $method
    ) {
      $value = $this->getSetting($key);
      if ($value !== NULL && $value !== '') {
        $layout->{$method}($value);
      }
    }
    return [$layout];
  }

}

/**
 * Base class for component layout destinations.
 *
 * All ComponentLayout destinations must extend this class.
 */
class ApplenewsDestinationComponentLayout extends ApplenewsDestination {

  /**
   * Destination object(s), to be returned by export().
   *
   * @var \ChapterThree\AppleNewsAPI\Document\Layouts\ComponentLayout
   *   ComponentLayout object.
   */
  public $destination;

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
  public function getConfigForm(array $values = []) {
    $form = parent::getConfigForm($values);

    $form['id'] = [
      '#title'         => t('ID'),
      '#type'          => 'textfield',
      '#default_value' => $this->id,
      '#required'      => TRUE,
    ];

    $form['settings']['columnStart'] = [
      '#title'         => t('Column Start'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('columnStart'),
    ];

    $form['settings']['columnSpan'] = [
      '#title'         => t('Column Span'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('columnSpan'),
    ];

    $form['settings']['margin'] = [
      '#title' => t('Margin'),
      '#type' => 'fieldset',
      'type' => [
        '#type' => 'select',
        '#options' => [
          'value' => t('Integer value for all sides'),
          'margin' => t('Separate values for top and bottom margins.'),
        ]
      ],
      'value' => [],
      'margin' => [],
    ];

    $form['settings']['contentInset'] = [
      '#title'         => t('Content Inset'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('contentInset'),
    ];

    $form['settings']['ignoreDocumentMargin'] = [
      '#title'         => t('Ignore Document Margin'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('ignoreDocumentMargin'),
    ];

    $form['settings']['ignoreDocumentGutter'] = [
      '#title'         => t('Ignore Document Gutter'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('ignoreDocumentGutter'),
    ];

    $form['settings']['minimumHeight'] = [
      '#title'         => t('Minimum Height'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('minimumHeight'),
    ];

    $form['settings']['maximumContentWidth'] = [
      '#title'         => t('Maximum Content Width'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('maximumContentWidth'),
    ];

    $form['settings']['horizontalContentAlignment'] = [
      '#title'         => t('Horizontal Content Alignment'),
      '#type'          => 'textfield',
      '#default_value' => $this->getSetting('horizontalContentAlignment'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigForm($values) {
    parent::validateConfigForm($values);
    if ($values['id'] != $this->id &&
        $this->export->getComponentLayout($values['id'])
    ) {
      form_set_error('id', t('ID already in use.'));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigForm($values) {
    parent::submitConfigForm($values);
    if ($values['id'] != $this->id) {
      $this->export->deleteComponentLayout($this->id);
      $this->id = $values['id'];
      $this->export->addComponentLayout($this, $this->id);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
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
      if ($value !== NULL && $value !== '') {
        $layout->{$method}($value);
      }
    }
    return [$layout];
  }

}
