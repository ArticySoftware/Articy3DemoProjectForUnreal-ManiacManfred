<p align="center">
  <img src="https://www.articy.com/articy-importer/unity/media/ManiacManfred_Title.png">
</p>

The "Maniac Manfred" demo project is a full articy:draft project we shipped with articy:draft and referenced it in our online help and in some of our product videos. In addition to our existing Unity Maniac Manfred demo project we decided to create an Unreal demo project too.  
This project is a good way to show you how you can create a full game using articy:draft and bring it to life in Unreal using the ArticyImporter plugin.

# Setup and installation

Before you can start you need to make sure you have all the additional applications and projects downloaded:

* Unreal Engine 5.xx (this demo project is also available for Unreal 4.25.x [here](https://github.com/ArticySoftware/DemoProjectForUnreal-ManiacManfred/tree/UE4))
* Visual Studio 2017, 2019 or 2022
* articy:draft 3.1.16 or higher
* The "Maniac Manfred" articy:draft demo project, which you can get [here](https://www.articy.com/redirect/Manfred.Articy)

You only need the articy:draft Maniac Manfred project, if you want to make changes to the project and export them to see how the Unreal project and the plugin behaves.

# Overview

The main goal of Maniac Manfred is to show you how you can use articy:draft's features for planing, designing, and creating a game and then use Unreal and the ArticyImporter Plugin to bring it to life.

But it should be noted that Maniac Manfred is by no means a final game, or the one and only way how you could create an adventure. In fact the code misses a lot of performance optimization for the sake of easier understanding. For a different game, even for a different adventure a complete different design in articy and Unreal would probably make more sense.

Maniac Manfred features:

* Simple point and click game
* Inventory system with item combination
* Interactive branching dialogue system
* Multiple scenes
* Moral system and different endings
* Localization support

Maniac Manfred was not build to win any prizes or awards. It lacks a lot of polishing, and is better to be considered a prototype or proof of concept. But we still hope you have some fun playing it and while digging through the project you get some ideas how to create your own games!

# How to get started

The best way to get started is to take a look into the `LevelHandler` Blueprint. Besides of the general game management of Maniac Manfred, you can see how to traverse through the flow in the form of dialogues with the ArticyFlowPlayer component, deal with articy objects and implement script methods created in articy.  
Another instructive point of the project is the inventory system, which is split into the Blueprints `Inventory` and `InventoryItem`. Both show much about how to deal with global variables, features and templates.  
Regarding the usage of articy conditions and instructions the `ClickableZone` and the `LevelImageElement` are the most interesting Blueprints.  
If you want to learn more about the structure of the articy:draft project itself, you can take a look into [this](https://www.articy.com/articy-importer/unity/html/howto_maniacmanfred.htm) article about the Unity Maniac Manfred project.
