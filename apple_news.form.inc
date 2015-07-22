<?php
/**
 * @file
 * Form hook implementations.
 */

/**
 * Implements hook_form_alter().
 */
function apple_news_form_alter(&$form, &$form_state, $form_id) {
  module_load_include('inc', 'apple_news');
  // Get all possible entity types and bundles available for export.
  $entity_types = apple_news_get_entity_types();
  foreach ($entity_types as $type => $bundles) {
  	foreach ($bundles as $bundle) {
      if ($form_id == $bundle . '_' . $type . '_form') {
      	// Add Apple News vertical tab.
        $form['apple_news'] = array(
          '#type' => 'fieldset',
          '#title' => t('Apple News'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#group' => 'additional_settings',
          '#weight' => -10,
        );
        // Add Post to Apple News checkbox.
        $form['apple_news']['post'] = array(
          '#type' => 'checkbox',
          '#title' => t('Post to Apple News'),
          '#description' => t('Post this <em>@entity_type</em> to Apple News.',
          	array('@entity_type' => $type)
          )
        );
      }
    }
  }

}
