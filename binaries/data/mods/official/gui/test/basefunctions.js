function GUIObjectHide(objectName) {

        // Hide our GUI object
        GUIObject = getGUIObjectByName(objectName);
        GUIObject.hidden = true;

}

function GUIObjectUnhide(objectName) {

        // Unhide our GUI object
        GUIObject = getGUIObjectByName(objectName);
        GUIObject.hidden = false;

}

function GUIObjectToggle(objectName) {

        // Get our GUI object
        GUIObject = getGUIObjectByName(objectName);

        // Toggle it
        GUIObject.hidden = !GUIObject.hidden;

}

function GUIUpdateObjectInfo() {

        // Get GUI Objects needed
        ObjectNameText = getGUIObjectByName("selection_name_test");
        ObjectPositionText = getGUIObjectByName("selection_pos_test");
        ObjectSpeedText = getGUIObjectByName("selection_speed_test");
	ObjectPortrait = getGUIObjectByName("selection_portrait_test");
	ObjectStatAttack = getGUIObjectByName("statistic_attack");
	ObjectStatHack = getGUIObjectByName("statistic_hack");
	ObjectStatPierce = getGUIObjectByName("statistic_pierce");
	ObjectStatAccuracy = getGUIObjectByName("statistic_accuracy");
	ObjectStatLOS = getGUIObjectByName("statistic_los");
	ObjectStatSpeed = getGUIObjectByName("statistic_speed");

        // Check number of selected entities
        if (selection.length > 1) {

                var MultipleEntitiesSelected = true;
                ObjectNameText.caption = "Selected " + selection.length + " entities.";
                ObjectNameText.hidden = false;
                ObjectPositionText.hidden = true;
                ObjectSpeedText.hidden = true;

        } else {

                if (!selection[0]) {

			// Reset portrait
			ObjectPortrait.hidden = true;

			// Reset statistic icons.
			ObjectStatAttack.hidden = true;
			ObjectStatHack.hidden = true;
			ObjectStatPierce.hidden = true;
			ObjectStatAccuracy.hidden = true;
			ObjectStatLOS.hidden = true;
			ObjectStatSpeed.hidden = true;

                        // Reset object name
                        ObjectNameText.caption = "";
                        ObjectNameText.hidden = true;

                        // Reset position
                        ObjectPositionText.caption = "";
                        ObjectPositionText.hidden = true;

                        // Reset Speed
                        ObjectSpeedText.caption = "";
                        ObjectSpeedText.hidden = true;

                } else {

			// Update portrait (temporary if/else statement until entity icons are implemented).
			if (selection[0].name == "Prometheus Dude") ObjectPortrait.sprite="portrait_unit_dude_lrg";
			else
			if (selection[0].name == "Prometheus Dudette") ObjectPortrait.sprite="portrait_unit_dudette_lrg";
			else
			if (selection[0].name == "Deciduous Tree 1" || selection[0].name == "Deciduous Tree 2") ObjectPortrait.sprite="portrait_flora_deciduotree_lrg";
			else
			if (selection[0].name == "House") ObjectPortrait.sprite="portrait_structure_heleho_lrg";
			else
				ObjectPortrait.sprite="portrait_unknown_lrg";
			ObjectPortrait.hidden = false;

			// Turn on statistic icons.
			ObjectStatAttack.hidden = false;
			ObjectStatHack.hidden = false;
			ObjectStatPierce.hidden = false;
			ObjectStatAccuracy.hidden = false;
			ObjectStatLOS.hidden = false;
			ObjectStatSpeed.hidden = false;

                        // Update object name
                        ObjectNameText.caption = selection[0].name;
                        ObjectNameText.hidden = false;

                        // Update position
                        var strString = "" + selection[0].position;
                        ObjectPositionText.caption = "Position: " + strString.substring(20,strString.length-3);
                        //ObjectPositionText.caption = strString;
                        ObjectPositionText.hidden = false;

                        // Update speed
                        ObjectSpeedText.caption = selection[0].speed;
                        ObjectSpeedText.hidden = false;
                }
        
        }
        
}
