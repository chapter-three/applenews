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
  public function getSetting($key) {
    // Hard-coded inherited settings.
    switch ($key) {

      case 'layout_columns':
        return 7;

      case 'layout_width':
        return 1024;

    }
    if (isset($this->settings[$key])) {
      return $this->settings[$key];
    }
    // Defaults.
    switch ($key) {

      case 'layout_alignment':
        return 'left';

    }
    return parent::getSetting($key);
  }

  /**
   * Provides a configuration form for this export.
   */
  public function getConfigForm($form, &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    // Remove parent layout elements.
    $form['layout']['layout_columns']['#type'] = 'value';
    $form['layout']['layout_width']['#type'] = 'value';

    $form['layout']['layout_alignment'] = [
      '#type'          => 'select',
      '#title'         => t('Alignment'),
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
   * {@inheritdoc}
   */
  public function submitConfigForm($form, &$form_state) {
    $this->setSetting('layout_alignment',
      $form_state['values']['layout_alignment']);
    switch ($this->getSetting('layout_alignment')) {

      case 'left':
      case 'right':
        $this->setSetting('layout_width', 1024);
        $this->setSetting('layout_columns', 7);
        break;

      case 'center':
        $this->setSetting('layout_width', 768);
        $this->setSetting('layout_columns', 9);
        break;

    }
    parent::submitConfigForm($form, $form_state);
  }

  /**
   * Set any relations between components.
   */
  protected function associateComponents() {

    $components = $this->document->getComponents();
    for ($i = 0; $i < count($components); $i++) {

      /** @var \ChapterThree\AppleNews\Document\Components\Component $comp */
      $comp = $components[$i];

      // Associate with previous component.
      if ($i) {
        /** @var \ChapterThree\AppleNews\Document\Components\Component $prev */
        $prev = $components[$i - 1];

        // Anchor to previous component.
        $anchor = FALSE;
        if ($comp instanceof Components\ScalableImage) {
          if ($prev instanceof Components\Body) {
            $anchor = TRUE;
          }
        }
        elseif ($comp instanceof Components\Pullquote) {
          $anchor = TRUE;
        }
        elseif ($comp instanceof Components\ComponentNested) {
          /** @var \ChapterThree\AppleNews\Document\Components\ComponentNested $comp */
          $class_name = '\ChapterThree\AppleNews\Document\Components\Pullquote';
          if ($comp->hasComponentType($class_name)) {
            $anchor = TRUE;
          }
        }
        if ($anchor) {
          if (!$id = $prev->getIdentifier()) {
            $id = $this->document->generateIdentifier();
            $prev->setIdentifier($id);
          }
          $anchor = new Anchor('bottom');
          $anchor->setTargetComponentIdentifier($id)
            ->setRangeStart(500)
            ->setRangeLength(1);
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
    $this->document->getLayout()
      ->setMargin(75)
      ->setGutter(20);
    $this->associateComponents();
    return $this->document;
  }

}
