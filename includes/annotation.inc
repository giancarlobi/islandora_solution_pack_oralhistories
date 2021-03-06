<?php
//function edit_annotation_modal(AbstractObject $object, $ajax) {
//  if (!$object) {
//    $path = current_path();
//    $part_parts = explode('/', $path);
//    $object = islandora_object_load($part_parts[2]);
//  }
//  if ($ajax) {
//    ctools_include('ajax');
//    ctools_include('modal');
//
//    $form_state = array(
//      'ajax' => TRUE,
//      'title' => t('Annotation Modal Form'),
//    );
//    // pass arguments to ctools_modal_form_wrapper().
//    $form_state['build_info']['args'][] = $object;
//
//    // Use ctools to generate ajax instructions for the browser to create
//    // a form in a modal popup.
//    $output = ctools_modal_form_wrapper(
//      'islandora_oralhistories_annotation_form',
//      $form_state
//    );
//
//    if (!empty($form_state['executed'])) {
//
//      // Add the responder javascript, required by ctools
//      ctools_add_js('ajax-responder');
//
//      // Create ajax command array, dismiss the modal window.
//      $output = array();
//      $output[] = ctools_modal_command_dismiss();
//    }
//
//
//    // Return the ajax instructions to the browser via ajax_render().
//    print ajax_render($output);
//    drupal_exit();
//  }
//  else {
//    return drupal_get_form(
//      'islandora_oralhistories_annotation_form',
//      $object
//    );
//  }
//}

function _get_web_annotation_form(AbstractObject $object) {
  $form = drupal_render(drupal_get_form('islandora_oralhistories_annotation_form', $object));
  $javascript = drupal_add_js(NULL, NULL);
  $settings = FALSE;

  if(isset($javascript['settings'])) {
    $settings = '<script type="text/javascript">jQuery.extend(Drupal.settings, ' .
      drupal_json_encode(call_user_func_array('array_merge_recursive', $javascript['settings']['data'])) .
      ');</script>';
  }

  die($form . $settings);
}

/**
 * Menu callback for "islandora/object/%islandora_object/web_annotation".
 *
 * @param \AbstractObject $object
 * @return string
 */
function _retrieve_web_annotations(AbstractObject $object) {
  // @todo: retrieve all annotations for a given $object, return in json.
  return 'Annotation list';
}

/**
 * Menu callback for "islandora/object/%islandora_object/web_annotation/create"
 *
 * Our annotationApp will post data to this url.
 * @return string
 */
function _create_web_annotation() {
  // @todo: add a new annotation.
  // Retrieve our POST DATA

  
  return 'A new annotation is added.';
}

/**
 * Menu callback for "islandora/object/%islandora_object/web_annotation/remove/%"
 * @param \AbstractObject $object
 * @param string $anno_object_pid
 * @return string
 */
function _remove_web_annotation(AbstractObject $object, $anno_object_pid = '') {
  // @todo: remove an existing annotation.
  return 'remove an existing annotation.';
}

/**
 * Menu callback for "islandora/object/%islandora_object/web_annotation/remove/%"
 * @param \AbstractObject $object
 * @param string $anno_object_pid
 * @return string
 */
function _update_web_annotation(AbstractObject $object, $anno_object_pid = '') {
  // @todo: update an existing annotation.
  return 'update an existing annotation.';
}


function islandora_oralhistories_annotation_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora', 'includes/datastream');
  $form = array();
  $form['#tree'] = TRUE;
  $form['#prefix'] = '<div id="islandora-oralhistories-annotation-form-wrapper">';
  $form['#suffix'] = '</div>';

  $form['web_annotation'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="web-annotation-container-wrapper">',
    '#suffix' => '</div>',
  );

  $form['web_annotation']['start'] = array(
    '#type' => 'textfield',
    '#title' => t('Start time'),
    '#description' => t('Start time in seconds'),
    '#default_value' => 0.00,
    '#size' => 25,
    '#attributes' => array(
      'class' => array('web-annotation-start'),
      'id' => 'web-annotation-start-time'
    ),
  );
  $form['web_annotation']['end'] = array(
    '#type' => 'hidden',
    '#title' => t('End time'),
    '#description' => t('End time in seconds'),
    '#size' => 25,
    '#attributes' => array(
      'class' => array('web-annotation-end'),
      'id' => 'web-annotation-end-time'
    ),
  );
  $form['web_annotation']['annotation_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Annotation content'),
    '#description' => t('Annotation content'),
    '#attributes' => array(
      'class' => array('web-annotation-body'),
      'id' => 'web-annotation-body'
    ),
  );
  $form['web_annotation']['annotation_motivation'] = array(
    '#type' => 'select',
    '#title' => t('Motivation'),
    '#description' => t('Reasons to create an annotation'),
    '#options' => array(
      'oa:commenting' => t('Commenting'),
      'oa:describing' => t('Describing'),
      'oa:replying' => t('Replying'),
      'oa:reviewing' => t('Reviewing'),
      'oa:tagging' => t('Tagging'),
    ),
    '#default_value' => 'oa:commenting',
    '#attributes' => array(
      'class' => array('web-annotation-motivation'),
      'id' => 'web-annotation-motivation'
    ),
  );
  $form['web_annotation']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    "#ajax" => array(
      "callback" => "web_annotation_form_ajax_callback",
      "wrapper" => "islandora-oralhistories-annotation-form-wrapper",
    ),
  );

  return $form;
}

function web_annotation_form_ajax_callback(array $form, array &$form_state) {
  if (form_get_errors()) {
    $form_state['rebuild'] = TRUE;
    $commands = array();
    $commands[] = ajax_command_prepend(NULL, theme('status_messages'));
    return array('#type' => 'ajax', '#commands' => $commands);
  }
  else {
    $system_message = drupal_get_messages();
    return t('Thank you for your submission!');
  }

}
