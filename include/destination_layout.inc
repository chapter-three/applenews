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
   *
   * @return \ChapterThree\AppleNewsAPI\Document\Layouts\Layout
   *   Apple News Document layout object.
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
      '#title'         => t('Identifier'),
      '#description'   => t('Use a short, descriptive string. This will be used to select this layout on the component configuration page.'),
      '#type'          => 'textfield',
      '#default_value' => $this->id,
      '#required'      => TRUE,
      '#weight'        => 1,
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
      '#pre_render' => ['applenews_form_pre_render_margin'],
      'type' => [
        '#type' => 'select',
        '#options' => [
          'value' => t('Integer value for all sides'),
          'margin' => t('Separate values for top and bottom margins'),
        ],
        '#default_value' => @$this->getSetting('margin')['type'],
      ],
      'value' => [
        '#title'         => t('Value'),
        '#type'          => 'textfield',
        '#default_value' => @$this->getSetting('margin')['value'],
      ],
      'top' => [
        '#title'         => t('Top'),
        '#type'          => 'textfield',
        '#default_value' => @$this->getSetting('margin')['top'],
      ],
      'bottom' => [
        '#title'         => t('Bottom'),
        '#type'          => 'textfield',
        '#default_value' => @$this->getSetting('margin')['bottom'],
      ],
    ];

    $form['settings']['contentInset'] = [
      '#title'         => t('Content Inset'),
      '#type' => 'fieldset',
      '#pre_render' => ['applenews_form_pre_render_content_inset'],
      'type' => [
        '#type' => 'select',
        '#options' => [
          'default' => t('Default Inset'),
          'contentInset' => t('Define each side'),
        ]
      ],
      'default' => [
        '#title'         => t('Enable'),
        '#type'          => 'checkbox',
        '#default_value' => @$this->getSetting('contentInset')['default'],
      ],
      'top' => [
        '#title'         => t('Top'),
        '#type'          => 'checkbox',
        '#default_value' => @$this->getSetting('margin')['top'],
      ],
      'right' => [
        '#title'         => t('Right'),
        '#type'          => 'checkbox',
        '#default_value' => @$this->getSetting('margin')['right'],
      ],
      'bottom' => [
        '#title'         => t('Bottom'),
        '#type'          => 'checkbox',
        '#default_value' => @$this->getSetting('margin')['bottom'],
      ],
      'left' => [
        '#title'         => t('Left'),
        '#type'          => 'checkbox',
        '#default_value' => @$this->getSetting('margin')['left'],
      ],
    ];

    $form['settings']['ignoreDocumentMargin'] = [
      '#title'         => t('Ignore Document Margin'),
      '#type'          => 'select',
      '#options'       => [
        ''      => t('No'),
        'left'  => t('Left'),
        'right' => t('Right'),
        'both'  => t('Both'),
      ],
      '#default_value' => $this->getSetting('ignoreDocumentMargin'),
    ];

    $form['settings']['ignoreDocumentGutter'] = [
      '#title'         => t('Ignore Document Gutter'),
      '#type'          => 'select',
      '#options'       => [
        ''      => t('No'),
        'left'  => t('Left'),
        'right' => t('Right'),
        'both'  => t('Both'),
      ],
      '#default_value' => $this->getSetting('ignoreDocumentGutter'),
    ];

    $form['settings']['minimumHeight'] = [
      '#title'         => t('Minimum Height'),
      '#type'          => 'applenews_supported_units',
      '#default_value' => $this->getSetting('minimumHeight'),
    ];

    $form['settings']['maximumContentWidth'] = [
      '#title'         => t('Maximum Content Width'),
      '#type'          => 'applenews_supported_units',
      '#default_value' => $this->getSetting('maximumContentWidth'),
    ];

    $form['settings']['horizontalContentAlignment'] = [
      '#title'         => t('Horizontal Content Alignment'),
      '#type'          => 'select',
      '#options'       => [
        ''        => t('None'),
        'center'  => t('Center'),
        'left'    => t('Left'),
        'right'   => t('Right'),
      ],
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
    if (@$values['id'] != $this->id) {
      // Change any component references to this layout.
      /** @var ApplenewsDestinationComponent $component */
      foreach ($this->export->getComponents() as $component) {
        $layout = $component->getLayout();
        if ($layout && $layout->id == $this->id) {
          $component->setLayout($values['id']);
        }
      }
      // Save identifier as id attribute -- export class allows this to be an
      // arbitrary array key.
      // @see ApplenewsExportNode::exportDocument()
      $this->export->deleteComponentLayout($this->id);
      $this->id = $values['id'];
      $this->export->addComponentLayout($this, $this->id);
    }
  }

  /**
   * {@inheritdoc}
   *
   * @return \ChapterThree\AppleNewsAPI\Document\Layouts\ComponentLayout
   *   Apple News Document componentLayout object.
   */
  public function export() {
    $layout = new ComponentLayout();

    // Simple values.
    foreach ([
        'columnStart'                => 'setColumnStart',
        'columnSpan'                 => 'setColumnSpan',
        'ignoreDocumentMargin'       => 'setIgnoreDocumentMargin',
        'ignoreDocumentGutter'       => 'setIgnoreDocumentGutter',
        'horizontalContentAlignment' => 'setHorizontalContentAlignment',
      ] as $key => $method
    ) {
      $value = $this->getSetting($key);
      if ($value !== NULL && $value !== '') {
        $layout->{$method}($value);
      }
    }

    $value = $this->getSetting('margin');
    switch ($value['type']) {

      case 'margin':
        $layout->setMargin(new \ChapterThree\AppleNewsAPI\Document\Margin(
          @$value['top'], @$value['bottom']));
        break;

      case 'value':
      default:
        if (!empty($value['value'])) {
          $layout->setMargin($value['value']);
        }
        break;

    }

    $value = $this->getSetting('contentInset');
    switch ($value['type']) {

      case 'contentInset':
        $inset = new \ChapterThree\AppleNewsAPI\Document\ContentInset();
        $inset->setTop(!empty($value['top']));
        $inset->setRight(!empty($value['right']));
        $inset->setBottom(!empty($value['bottom']));
        $inset->setLeft(!empty($value['left']));
        $layout->setContentInset($inset);
        break;

      case 'default':
      default:
        if (!empty($value['default'])) {
          $layout->setContentInset(TRUE);
        }
        break;

    }

    $value = $this->getSetting('minimumHeight');
    if (!empty($value['value'])) {
      $layout->setMinimumHeight($value['value'] . $value['units']);
    }

    $value = $this->getSetting('maximumContentWidth');
    if (!empty($value['value'])) {
      $layout->setMaximumContentWidth($value['value'] . $value['units']);
    }

    return $layout;
  }

}
