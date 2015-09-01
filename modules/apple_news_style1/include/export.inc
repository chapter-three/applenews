<?php

/**
 * @file
 * Export classes.
 */

use ChapterThree\AppleNews\Document;
use \ChapterThree\AppleNews\Document\Anchor;
use \ChapterThree\AppleNews\Document\Components;

/**
 * A configurable node export.
 */
class Applenewsstyle1ExportNode extends ApplenewsExportNode {

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'layout_alignment' => 'left',
    ] + parent::settings();
  }

  /**
   * Provides a configuration form for this export.
   */
  public function getConfigForm($form, &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    // Hide parent layout elements.
    $tab = &$form['additional_settings']['layouts_tab'];
    $tab['layout']['#access'] = FALSE;
    $tab['component_layouts']['#access'] = FALSE;

    $tab['layout_alignment'] = [
      '#type'          => 'select',
      '#title'         => t('Alignment'),
      '#required'      => TRUE,
      '#default_value' => $this->getSetting('layout_alignment'),
      '#options'       => [
        'left'   => t('Left'),
        'center' => t('Center'),
        'right'  => t('Right'),
      ],
    ];

    return $form;
  }

  /**
   * Set any relations between components.
   */
  protected function associateComponents() {

    $components = $this->document->getComponents();
    for ($i = 0; $i < count($components); $i++) {

      /** @var \ChapterThree\AppleNews\Document\Components\Component $comp */
      $comp = $components[$i];

      // Associate with previous body component.
      if ($i) {
        /** @var \ChapterThree\AppleNews\Document\Components\Body $prev */
        $prev = $components[$i - 1];
        if (!$prev instanceof Components\Body) {
          continue;
        }

        // Anchor to previous component.
        $anchor_range_length = FALSE;
        if ($comp instanceof Components\ScalableImage ||
            $comp instanceof Components\Pullquote
        ) {
          $anchor_range_length = strlen($prev->getText());
        }
        elseif ($comp instanceof Components\ComponentNested) {
          /** @var \ChapterThree\AppleNews\Document\Components\ComponentNested $comp */
          $class_name = '\ChapterThree\AppleNews\Document\Components\Pullquote';
          if ($comp->hasComponentType($class_name)) {
            $anchor_range_length = strlen($prev->getText());
          }
        }
        if ($anchor_range_length) {
          if (!$id = $prev->getIdentifier()) {
            $id = $this->document->generateIdentifier();
            $prev->setIdentifier($id);
          }
          $anchor = new Anchor('bottom');
          $anchor->setOriginAnchorPosition('bottom')
            ->setTargetComponentIdentifier($id)
            ->setRangeStart(0)
            ->setRangeLength($anchor_range_length);
          $comp->setAnchor($anchor);
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function export($entity) {
    parent::export($entity);

    $layout = $this->document->getLayout();
    switch ($this->getSetting('layout_alignment')) {

      case 'left':
      case 'right':
        $layout->setWidth(1024)->setColumns(7);
        break;

      case 'center':
        $layout->setWidth(768)->setColumns(9);
        break;

    }
    $layout->setMargin(75)->setGutter(20);
    $this->associateComponents();
    return $this->document;
  }

}
