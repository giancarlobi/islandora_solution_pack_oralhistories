<?php

/**
 * @file
 * Theme hooks.
 */

/**
 * Implements hook_preprocess().
 */
function islandora_oralhistories_preprocess_islandora_oralhistories(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora', 'includes/authtokens');
  module_load_include('inc', 'islandora', 'includes/metadata');
  module_load_include('inc', 'islandora_oralhistories', 'includes/utilities');
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  $module_path = drupal_get_path('module', 'islandora_oralhistories');

  // Add required bootstrap-select css and js files.
  drupal_add_css('//cdnjs.cloudflare.com/ajax/libs/bootstrap-select/1.6.4/css/bootstrap-select.min.css', 'external');
  drupal_add_js('//cdnjs.cloudflare.com/ajax/libs/bootstrap-select/1.6.4/js/bootstrap-select.min.js', 'external');

  // Add videojs.
  drupal_add_css($module_path . '/lib/videojs/video-js.css');
  drupal_add_js($module_path . '/lib/videojs/videojs.js');


  // Use jquery.ui tabs.
  drupal_add_library('system', 'ui.tabs');

  $object = $variables['object'];
  $repository = $object->repository;
  // We should eventually remove the DC object and dc_array code as it only
  // exists to not break legacy implementations.
  try {
    if (isset($object['DC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['DC'])) {
      $dc = $object['DC']->content;
      $dc_object = DublinCore::importFromXMLString($dc);
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
  $variables['dc_array'] = isset($dc_object) ? $dc_object->asArray() : array();
  $variables['islandora_object_label'] = $object->label;
  $variables['theme_hook_suggestions'][] = 'islandora_oralhistories__' . str_replace(':', '_', $object->id);
  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($object);
  $variables['metadata'] = islandora_retrieve_metadata_markup($object);
  $variables['description'] = islandora_retrieve_description_markup($object);

  // construct $variables['islandora_content'] dynamically.
  $variables['islandora_content'] = array();
  $viewer_params = array();

  // prepare viewer params for media file.
  try {
    $mime = 'video/mp4';
    $viewer_params = array(
      'pid' => $object->id,
    );
    $viewer_params += array(
      'media_tag' => 'video',
    );
    if (isset($object['MP4'])) {
      $media_dsid = 'MP4';
    }
    if (!$object['MP4']) {
      if (variable_get('islandora_video_play_obj', true)) {
        if ($object['OBJ'] && $object['OBJ']->mimetype == 'video/mp4') {
          $max_file_size = ((int) variable_get('islandora_video_max_obj_size', 500)) * 1024 * 1024;
          if ($object['OBJ']->size < $max_file_size) {
            $media_dsid = 'OBJ';
          }
        }
      }
    }

    // Video player.
    if (isset($object[$media_dsid]) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object[$media_dsid])) {
      $media_url = url("islandora/object/{$object->id}/datastream/$media_dsid/view", array('absolute' => TRUE));
      $viewer_params += array(
        'mime' => $mime,
        'url' => $media_url,
      );
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retrieving datastream %s %t', array('%s' => $media_dsid, '%t' => $e->getMessage())), 'error', FALSE);
  }

  try {
    if (isset($object['PROXY_MP3']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['PROXY_MP3'])) {
      $media_dsid = 'PROXY_MP3';
      $mime = 'audio/mpeg';

      $viewer_params = array(
        'pid' => $object->id,
      );
      $viewer_params += array(
        'media_tag' => 'audio',
      );
      // Audio player.
      if (isset($object[$media_dsid]) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object[$media_dsid])) {
        $media_url = url("islandora/object/{$object->id}/datastream/$media_dsid/view", array('absolute' => TRUE));
        $viewer_params += array(
          'mime' => $mime,
          'url' => $media_url,
        );
      }
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retrieving datastream %s %t', array('%s' => $media_dsid, '%t' => $e->getMessage())), 'error', FALSE);
  }

  // Thumbnail.
  if (isset($object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['TN'])) {
    $viewer_params += array(
      'tn' => url("islandora/object/{$object->id}/datastream/TN/view", array('absolute' => TRUE)),
    );
  }

  // get all transcript datastreams if more than one?
  $tracks= array();
  $transcript_mimetype = '';
  foreach ($object as $datastream) {
    if (preg_match('/TRANSCRIPT/', $datastream->id)) {
      if (strpos($datastream->mimetype,'xml') !== false) {
        $transcript_mimetype = 'xml';
      }
      else {
        $transcript_mimetype = 'vtt';
      }
    }
    if (preg_match('/MEDIATRACK/', $datastream->id)) {
      $track = array();

      $source_url = url("islandora/object/{$object->id}/datastream/$datastream->id/view", array('absolute' => TRUE));

      // @todo: how to determine language code? default to 'en'.
      $lang_code = 'en';

      $track['dsid'] = $datastream->id;
      $track['lang_code'] = $lang_code;
      $track['source_url'] =  $source_url;


      if (isset($object[$datastream->id]) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object[$datastream->id])) {
        $tracks[] = $track;
      }
    }
  }
  if (!empty($tracks)) {
    $viewer_params += array(
      'tracks' => $tracks,
    );
  }

  if (variable_get('islandora_oralhistories_enbable_transcript_display') && !empty($tracks)) {
    foreach ($tracks as $track) {
      switch ($transcript_mimetype) {
        case 'vtt':
          $viewer_params += array(
            'transcript_content' => vtt_transcript_content($track['source_url']),
          );
          break;

        case 'xml':
          $trid = str_replace(':', '-', $object->id);
          $viewer_params += array(
            'transcript_content' => xml_transcript_content($trid),
          );
          break;
      }
    }

  }

  // Retrieve/display annotations.
  if (variable_get('islandora_oralhistories_annotation_tab_display', FALSE)) {
    $viewer_params += array(
      //@todo: we may need change this to function _retrieve_web_annotations().
      'annotation_content' => islandora_oralhistories_get_annotation($object->id),
    );
  }

  $viewer_params += array(
    'enable_caption_display' => variable_get('islandora_oralhistories_enable_caption_display'),
    'enable_transcript_display' => variable_get('islandora_oralhistories_enbable_transcript_display'),
  );

//  $viewer = theme('islandora_native_html5', array('params' => $viewer_params));

  if (variable_get('islandora_oralhistories_transcript_layout_side')) {
    $viewer = theme('islandora_html5_video_side', array('params' => $viewer_params));
  }
  else {
    $viewer = theme('islandora_html5_video_stack', array('params' => $viewer_params));
  }

  $variables['islandora_content']['viewer'] = $viewer;

  // Pass settings to javascript.
  $js_settings = array(
    'objectId' => $object->id,
    'enableTranscriptDisplay' => variable_get('islandora_oralhistories_enable_transcript_display'),
    'enableCaptionDisplay' => variable_get('islandora_oralhistories_enable_caption_display'),
    'enableAnnotationTabDisplay' => variable_get('islandora_oralhistories_annotation_tab_display'),
    'tracks' => $tracks
  );

  // Add more settings for annotation.
  global $user;
  $js_settings['permissions'] = array(
    'view' => islandora_oralhistories_web_annotation_access(array('view web annotations'), $object),
    'create' => islandora_oralhistories_web_annotation_access(array('create web annotations'), $object),
    'manage_own' => islandora_oralhistories_web_annotation_access(array('manage own web annotations'), $object),
    'manage_any' => islandora_oralhistories_web_annotation_access(array('manage any web annotations'), $object)
  );
  $js_settings['user'] = array(
    'uid' => $user->uid,
    'name' => ($user->uid == 0) ? 'anonymous' : $user->name
  );

  if ($object->models[0] == 'islandora:oralhistoriesCModel' ) {
    drupal_add_js(array('islandoraOralhistories' => $js_settings), 'setting');
    drupal_add_js(drupal_get_path('module', 'islandora_oralhistories') . '/js/oralhistories.js');
  }
  return array('' => $viewer);
}

/**
 * Implements hook_preprocess_HOOK().
 */
function islandora_oralhistories_preprocess_islandora_native_html5(array &$variables) {
  $params = $variables['params'];
  $variables['tn'] = $params['tn'];
  $variables['sources'][] = array('url' => $params['url'], 'mime' => $params['mime']);
  $variables['tracks'] = $params['tracks'];
  $variables['enable_caption_display'] = $params['enable_caption_display'];
  $variables['enable_transcript_display'] = $params['enable_transcript_display'];
  $variables['transcript_content'] = $params['transcript_content'];

}

/**
 * Implements hook_process_HOOK().
 */
function islandora_oralhistories_process_islandora_html5_video_side(&$variables) {
  $variables['params']['trid'] ='trid-' . str_replace(':', '-', $variables['params']['pid']); // trid-PID with : being replaced by -
}


/**
 * Implements hook_process_HOOK().
 */
function islandora_oralhistories_process_islandora_html5_video_stack(&$variables) {
  $variables['params']['trid'] ='trid-' . str_replace(':', '-', $variables['params']['pid']); // trid-PID with : being replaced by -
}

function islandora_oralhistories_preprocess_islandora_oralhistories_transcript_edit_form(array &$variables) {
//  dpm($variables, '$variables');
}

/**
 * Implements theme_islandora_oralhistories_transcript_edit_form().
 */
function theme_islandora_oralhistories_transcript_edit_form($variables) {
  // Renders all elements of a form.
  return drupal_render_children($variables['form']);
}

function theme_islandora_webannotation(array &$variables) {
  module_load_include('inc', 'islandora_oralhistories', 'includes/annotation');
  $path = current_path();
  $part_parts = explode('/', $path);
  $object = islandora_object_load($part_parts[2]);

  $out = "<div class='btn-group' role='group'>";
  if (isset($object) && islandora_object_access(ISLANDORA_WEB_ANNOTATION_CREATE, $object)) {
    $out .= "<div><a id='annotation-add-new-btn' class='btn btn-default add' title='Add' ";
    $out .= "'><span class='glyphicon glyphicon-plus'></span> " . t('Add new annotation') . "</a></div>";
    $out .= "<div id='web-annotation-form-ajax-loader'></div>";
  }
  $out .= "</div>";
  return $out;
}
