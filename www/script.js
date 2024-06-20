// Function to open a specific popup
function openPopup(popupId) {
    const popup = document.getElementById(popupId);
    popup.style.display = "flex"; // Show the popup
}

// Function to close a specific popup
function closePopup(popup) {
    popup.style.display = "none"; // Hide the popup
}

// Add event listeners to open buttons
document
    .getElementById("openPopupBtn1")
    .addEventListener("click", () => openPopup("popup1"));
document
    .getElementById("openPopupBtn2")
    .addEventListener("click", () => openPopup("popup2"));
document
    .getElementById("openPopupBtn3")
    .addEventListener("click", () => openPopup("popup3"));

// Add event listeners to close buttons
document.querySelectorAll(".close-btn").forEach((btn) => {
    btn.addEventListener("click", (event) => {
        const popup = event.target.closest(".popup-background");
        closePopup(popup);
    });
});

// Close the popup if the user clicks outside of the popup content
window.addEventListener("click", (event) => {
    document.querySelectorAll(".popup-background").forEach((popup) => {
        if (event.target === popup) {
            closePopup(popup);
        }
    });
});
