/**
 * @file
 * Apple News vertical tab helper.
 */

(function ($) {

  Drupal.behaviors.AppleNewsEntityForm = {
    attach: function(context) {
      // Display Apple News publish status in vertical tabs.
      $('fieldset.apple-news-options', context).drupalSetSummary(function (context) {
        if ($('.form-item-post-to-apple-news-channel input').attr('checked')) {
          return Drupal.t('Published to Apple News');
        }
      });
    }
  };

}(jQuery));
