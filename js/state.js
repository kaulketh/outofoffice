
export const room = {
    number: "",
    organization: "",
    department: "",
    transactionSupported: false
};

export const people = new Map();

export function resetRemoteState() {
    room.number = "";
    room.organization = "";
    room.department = "";
    people.clear();
    room.transactionSupported = false;
}
