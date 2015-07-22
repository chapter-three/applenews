<?php

/**
 * @file
 * Class for exporting Drupal content to Apple News.
 */

use ChapterThree\AppleNews\Document;

/**
 * Base class for exporting Drupal entities.
 */
abstract class AppleNewsExport {

  protected $module;
  protected $machineName;
  protected $arguments;
  public $enabled;
  public $highwater;
  public $settings;

  /**
   * Implement __construct().
   *
   * @param string $module
   *   The defining module.
   * @param string $machine_name
   *   The machine name of this object.
   * @param array $arguments
   *   Defined in hook_apple_news_api(), altered by hook_apple_news_api_alter().
   * @param bool $enabled
   *   Export is enabled.
   * @param string $highwater
   *   Highwater value for detecting updated content.
   * @param array $settings
   *   User-configurable settings, defined in $this->getConfigForm().
   *
   * @todo implement $highwater.
   */
  public function __construct($module, $machine_name, array $arguments, $enabled = FALSE, $highwater = NULL, array $settings = array()) {
    $this->module = $module;
    $this->machineName = $machine_name;
    $this->arguments = $arguments;
    $this->enabled = $enabled;
    $this->highwater = $highwater;
    $this->settings = $settings;
  }

  /**
   * Getter for module.
   */
  public function getModule() {
    return $this->module;
  }

  /**
   * Getter for machineName.
   */
  public function getMachineName() {
    return $this->machineName;
  }

  /**
   * Define exportable entities.
   *
   * @return EntityFieldQuery
   *   Object that will return exportable entities.
   */
  public function query() {
    $query = new EntityFieldQuery();
    $query->addMetaData('account', user_load(1));
    return $query;
  }

  /**
   * Provides a configuration form for this export.
   */
  public function getConfigForm($form, &$form_state) {

    $form['enabled'] = array(
      '#title' => t('Enabled'),
      '#type' => 'checkbox',
      '#default_value' => $this->enabled,
    );

    $channels = [];
    $api_channels = variable_get('apple_news_variable_api_channels', array());
    foreach ($api_channels as $channel_id => $info) {
      $channels[$channel_id] = $info['name'];
    }
    $form['channels'] = array(
      '#title' => t('Feeds'),
      '#type' => 'select',
      '#default_value' => $this->settings['channels'],
      '#multiple' => TRUE,
      '#options' => $channels,
      '#required' => TRUE,
    );

    return $form;
  }

  /**
   * Validation handler for the configuration form for this export.
   */
  public function validateConfigForm($form, &$form_state) {
  }

  /**
   * Submit handler for the configuration form for this export.
   */
  public function submitConfigForm($form, &$form_state) {
    $this->enabled = $form_state['values']['enabled'];
    $this->settings['channels'] = $form_state['values']['channels'];
  }

  /**
   * Exports the entity to an Apple Native Document.
   *
   * @param object $entity
   *   An entity to export.
   *
   * @return NULL|\ChapterThree\AppleNews\Document
   *   A document object, or NULL to skip the export.
   */
  public function export($entity) {
    return NULL;
  }

}
