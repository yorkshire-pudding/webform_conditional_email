<?php

/**
 * @file
 * Menu callbacks and functions for configuring conditional webform messages.
 */

/**
 * Get the node webform conditional components.
 *
 * @param object $node
 *   Node object.
 *
 * @return array
 *   Array of conditional components.
 */
function _webform_conditional_email_get_conditional_components($node) {
  if (!isset($node->webform) || empty($node->webform['components'])) {
    return array();
  }

  $conditional_components = array();
  foreach ($node->webform['components'] as $cid => $test_component) {
    if (webform_component_feature($test_component['type'], 'conditional')) {
      $conditional_components[$cid] = $test_component;
    }
  }

  return $conditional_components;
}

/**
 * Get an options array of operators.
 *
 * @return array
 *   Form API options array.
 */
function _webform_conditional_email_conditional_operators_options() {
  $operators = _webform_conditional_email_conditional_operators();
  foreach ($operators as &$operator) {
    $operator = $operator['label'];
  }

  return $operators;
}

/**
 * Build a conditional fieldset.
 *
 * @param int $n
 *   Fieldset id.
 * @param object $node
 *   Webform node object.
 * @param Array $conditional_components
 *   Webform conditional component array for node.
 * @param Array $conditional_operators
 *   WCC conditional operators options array.
 * @param object $wcc
 *   Webform conditional confirmation object.
 *
 * @return array
 *   Form API array.
 */
function _webform_conditional_email_conditional_fieldset($n, $node, $conditional_components, $conditional_operators, $wcc = NULL) {
  $defaults = array();
  if ($wcc) {
    $defaults = $wcc->conditionals;
  }

  $fieldset = array(
    '#type' => 'fieldset',
    '#title' => t('Conditional rule') . ' ' . ($n + 1),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('Create a rule to control when this confirmation message is used.'),
  );

  $fieldset['conditional_component_' . $n] = array(
    '#type' => 'select',
    '#title' => t('Component'),
    '#options' => webform_component_list($node, $conditional_components, FALSE, TRUE),
    '#description' => t('Select a component to decide when to use these confirmation message settings.'),
    '#default_value' => isset($defaults[$n]) ? $defaults[$n]['component'] : FALSE,
  );

  $fieldset['conditional_operator_' . $n] = array(
    '#type' => 'select',
    '#title' => t('Operator'),
    '#options' => $conditional_operators,
    '#description' => t('Determines whether the list below is inclusive or exclusive.'),
    '#default_value' => isset($defaults[$n]) ? $defaults[$n]['operator'] : FALSE,
  );

  $fieldset['conditional_values_' . $n] = array(
    '#type' => 'textarea',
    '#title' => t('Values'),
    '#description' => t('List values, one per line, that will trigger this action.'),
    '#default_value' => isset($defaults[$n]) ? $defaults[$n]['values'] : '',
  );

  return $fieldset;
}

/**
 * Validate handler; check the redirection settings.
 */
function webform_conditional_email_edit_confirmation_form_validate($form, &$form_state) {
  // Check that at least one conditional is set:
  if ($form_state['conditionals'] == 0 && $form_state['values']['conditionals']['conditional_values_0'] === '') {
    form_error($form['conditionals'][0]['conditional_values_0'], t('At least one conditional must be set.'));
  }
}

/**
 * Add conditional to the form.
 *
 * @param Array $form
 *   Form API array.
 * @param Array $form_state
 *   Form state.
 */
function _webform_conditional_email_edit_confirmation_form_add_conditional($form, &$form_state) {
  $n = $form_state['conditionals'];
  if ($form_state['values']['conditionals']['conditional_values_' . $n] === '') {
    form_error($form['conditionals'][$n]['conditional_values_' . $n], t('Check the last condition before adding a new one.'));
    return;
  }

  $form_state['conditionals']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Add conditional AJAX callback.
 *
 * @param Array $form
 * @param Array $form_state
 */
function _webform_conditional_email_edit_confirmation_form_add_conditional_callback($form, &$form_state) {
  return $form['conditionals'];
}

/**
 * Submit handler; save the new or updated confirmation message to the database.
 */
function webform_conditional_email_edit_confirmation_form_submit($form, &$form_state) {
  $conditionals = array();
  $n = 0;
  foreach ($form_state['values']['conditionals'] as $condition) {
    if (isset($condition['conditional_values_' . $n]) && $condition['conditional_values_' . $n] !== '') {
      $conditional = array();
      $conditional['component'] = $condition['conditional_component_' . $n];
      $conditional['operator'] = $condition['conditional_operator_' . $n];
      $conditional['values'] = $condition['conditional_values_' . $n];
      $conditionals[] = $conditional;
    }
    $n++;
  }

  $rule = new stdClass();
  $rule->nid = $form_state['values']['nid'];
  $rule->condition_id = $form_state['values']['eid'];
  $rule->conditional_weight = 0;

  $rule->conditionals = $conditionals;

  $condition_id = webform_conditional_email_message_save($rule);
  if ($condition_id) {
    drupal_set_message(t('Configuration saved.'));
  }
  else {
    drupal_set_message(t('There was a problem saving your configuration.'), 'error');
  }
}

/**
 * Submit handler; delete confirmation message.
 */
function webform_conditional_email_delete_confirmation_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];
  webform_conditional_email_message_delete($nid, $form_state['values']['condition_id']);
}
