# Proof of Concept for RPD Challenge Hackathon

This is the POC of the solution shown in the presentation. It takes an image of the path,
grayscale, resize, and normalize the image and feeds it to the neural network.
The neural network infers how accessible the path is from Level 1 to Level 5,
then sends that inference to the main receiver via ELTRES.

This was a WIP, only to show that it is possible to use Edge AI to lower transmission costs 
(power and hardware) of smart monitoring. ELTRES with Spresense is also in beta during the
hackathon so final submission will probably changed to something like LoRa or LTE.

The dataset was also using crumpled papers as "block" for demo purposes. In actuality, either collect
road images with varying levels of accessibility due to disasters or use some variety of background subtraction
algorithm.