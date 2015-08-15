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
    $this->associateComponents();
    return $this->document;
  }

}
