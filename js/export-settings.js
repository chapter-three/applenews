/**
 * @file
 * Apple News export settings vertical tab helper.
 */

(function ($) {

  Drupal.behaviors.AppleNewsExportSettings = {
    attach: function(context) {

      // General settings.
      $('fieldset#edit-general', context).drupalSetSummary(function (context) {
        if ($('input#edit-enabled:checked', context).length) {
          return Drupal.t('Export enabled');
        }
        else {
          return Drupal.t('Export disabled');
        }
      });

      // Metadata
      $('fieldset#edit-metadata', context).drupalSetSummary(function (context) {
        if ($('#edit-channels input:checked', context).length !== 0) {
          return Drupal.t('@count default sections. @is_preview', {
            '@count' : $('#edit-channels input:checked', context).length,
            '@is_preview' : $('.form-item-is-preview input:checked', context).length
              ? Drupal.t('Preview only')
              : Drupal.t('Visible to everyone')
          });
        }
        else {
          return Drupal.t('Default sections and content visibility');
        }
      });

      // Layout
      $('fieldset#edit-layouts-tab', context).drupalSetSummary(function (context) {
        if ($('.form-item', context).length !== 0) {
          return Drupal.t('Configure layout settings');
        }
        else {
          return Drupal.t('No layout settings');
        }
      });

      // Components
      $('fieldset#edit-components-tab', context).drupalSetSummary(function (context) {
        var count = $('table tbody tr', context).length - 1;
        if (count) {
          var commentCount = Drupal.formatPlural(count, '1 component enabled', '@count components enabled');
          return commentCount;
        }
        else {
          return Drupal.t('Please add components');
        }
      });

    }
  };

}(jQuery));
