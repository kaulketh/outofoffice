
export const room = {
    number: "",
    organization: "",
    department: ""
};

export const people = new Map();

export function resetRemoteState() {
    room.number = "";
    room.organization = "";
    room.department = "";
    people.clear();
}
