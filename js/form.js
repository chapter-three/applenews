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
      $('input[data-channel-id]').each(function() {
        var _channel_id = $(this).data('channel-id');
        if (!$(this).attr('checked')) {
          $('input[data-section-of="' + _channel_id + '"]').parent().hide();
        }
        $(this).click(function() {
          $('input[data-section-of="' + _channel_id + '"]').parent().toggle();
        });
      });
      $('.apple-news-options .sections').parent().css({'margin-left' : '20px'});
    }
  };

}(jQuery));
