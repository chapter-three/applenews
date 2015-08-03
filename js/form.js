/**
 * @file
 * Apple News vertical tab helper.
 */

(function ($) {

  Drupal.behaviors.AppleNewsEntityForm = {
    attach: function(context) {
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
    }
  };

}(jQuery));
