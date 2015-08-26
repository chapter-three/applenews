/**
 * @file
 * Apple News vertical tab helper.
 */

(function ($) {

  Drupal.behaviors.AppleNewsEntityForm = {
    attach: function(context) {

      // Channels and sections checkboxes.
      $('input[data-channel-id]').each(function() {
        var _channel_id = $(this).data('channel-id');
        if (!$(this).attr('checked')) {
          $('input[data-section-of="' + _channel_id + '"]').parent().hide();
        }
        $(this).click(function() {
          $('input[data-section-of="' + _channel_id + '"]').parent().toggle();
        });
      });
      $('.apple-news-sections').parent().css({'margin-left' : '20px'});

      // Vertical tab summary.
      $('fieldset.apple-news-options', context).drupalSetSummary(function (context) {

        if ($('.form-item-apple-news-publish-flag input:checked', context).length) {
          var $postdate = $('.apple-news-post-date', context);
          if ($postdate[0]) {
            return Drupal.t('Published on ' + $postdate.html());
          }
          else {
            return Drupal.t('Published');
          }
        }
        else {
          return Drupal.t('Not published');
        }

      });
    }
  };

}(jQuery));
